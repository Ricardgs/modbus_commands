/*
 * drv_modbus.c
 *
 *  Created on: Jan 24, 2026
 *      Author: ricard
 */
#include <stdbool.h>
#include "drv_modbus.h"
#include "drv_modbus_registers.h"
#include "status.h"
#include "error.h"

/* Macros */

#define DRV_MODBUS_BROADCAST_ADDRESS					0x00

#define DRV_MODBUS_FUNCTION_CODE_READ_HOLDING_REGS		0x03
#define DRV_MODBUS_FUNCTION_CODE_READ_INPUT_REGS		0x04
#define DRV_MODBUS_FUNCTION_CODE_WRITE_SINGLE_REG		0x06
#define DRV_MODBUS_FUNCTION_CODE_WRITE_MULTIPLE_REGS	0x10

#define DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_FUNCTION		0x01
#define DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_ADDRESS	0x02
#define DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_VALUE	0x03

#define DRV_MODBUS_TIMEOUT_BETWEEN_BYTES_MS				2
#define DRV_MODBUS_TIME_BETWEEN_FRAMES_MS				5

#define DRV_MODBUS_MAX_FRAME_LEN_BYTES					100

/* Type definitions */

typedef struct
{
	uint8_t num_holding_regs;
	uint8_t num_input_regs;
	uint16_t *holding_regs_val;
	uint16_t *input_regs_val;
	const uint16_t *holding_regs_addr;
	const uint16_t *input_regs_addr;
} drv_modbus_regs_s;

typedef enum
{
	DRV_MODBUS_STATE_IDLE,
	DRV_MODBUS_STATE_RECEIVING,
	DRV_MODBUS_STATE_CHECK_CRC,
	DRV_MODBUS_STATE_CHECK_FC,
	DRV_MODBUS_STATE_READ_HOLDING_REGS,
	DRV_MODBUS_STATE_READ_INPUT_REGS,
	DRV_MODBUS_STATE_WRITE_SINGLE_REG,
	DRV_MODBUS_STATE_WRITE_MULTIPLE_REGS,
	DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE,
	DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE,
	DRV_MODBUS_STATE_SEND_RESPONSE
} drv_modbus_state_e;

/* Local variables */

static drv_modbus_regs_s vdrv_modbus_regs[DRV_MODBUS_INST_MAX];
static uint8_t vdrv_modbus_addr[DRV_MODBUS_INST_MAX];
static status_e vdrv_modbus_status[DRV_MODBUS_INST_MAX] =
{
		STATUS_NOT_INIT
};
static hal_timer_timer_s vdrv_modbus_timer[DRV_MODBUS_INST_MAX];
static hal_timer_timer_inst_e drv_modbus_timer_inst[DRV_MODBUS_INST_MAX];
static drv_modbus_state_e vdrv_modbus_state[DRV_MODBUS_INST_MAX];
static hal_uart_uart_num_e drv_modbus_uart_inst[DRV_MODBUS_INST_MAX];
static uint8_t drv_modbus_frame_index[DRV_MODBUS_INST_MAX];
static uint8_t drv_modbus_frame_buffer[DRV_MODBUS_INST_MAX][DRV_MODBUS_MAX_FRAME_LEN_BYTES];

/* Local function declarations */

static void drv_modbus_crc_calc(uint8_t *buff, uint16_t len, uint8_t crc_buff[2]);

/* Initialize variables */

void drv_modbus_init(void)
{
	vdrv_modbus_regs[DRV_MODBUS_INST_0].num_holding_regs = DRV_MODBUS_0_HOLDING_REG_MAX;
	vdrv_modbus_regs[DRV_MODBUS_INST_0].num_input_regs = DRV_MODBUS_0_INPUT_REG_MAX;
	vdrv_modbus_regs[DRV_MODBUS_INST_0].holding_regs_val = vdrv_modbus_0_holding_regs_val;
	vdrv_modbus_regs[DRV_MODBUS_INST_0].input_regs_val = vdrv_modbus_0_input_regs_val;
	vdrv_modbus_regs[DRV_MODBUS_INST_0].holding_regs_addr = vdrv_modbus_0_holding_regs_addr;
	vdrv_modbus_regs[DRV_MODBUS_INST_0].input_regs_addr = vdrv_modbus_0_input_regs_addr;

	for(drv_modbus_inst i = 0; i < DRV_MODBUS_INST_MAX; i++)
	{
		/* Ensure that the default address is not a valid address */
		vdrv_modbus_addr[i] = DRV_MODBUS_BROADCAST_ADDRESS;

		vdrv_modbus_status[i] = STATUS_NOT_STARTED;

		hal_timer_detach(&vdrv_modbus_timer[i]);

		vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

		drv_modbus_frame_index[i] = 0;
	}
}

/* Configure */

void drv_modbus_start(const drv_modbus_config_s config)
{
	if((config.inst < DRV_MODBUS_INST_MAX) && (vdrv_modbus_status[config.inst] == STATUS_NOT_STARTED))
	{
		vdrv_modbus_addr[config.inst] = config.mb_addr;

		drv_modbus_uart_inst[config.inst] = config.uart_inst;

		drv_modbus_timer_inst[config.inst] = config.timer_inst;

		vdrv_modbus_status[config.inst] = STATUS_STARTED;
	}
}

/* Fxn */

void drv_modbus_fxn(void)
{
	uint8_t crc[2];
	uint16_t reg_index;
	uint16_t j;
	uint16_t requested_address;
	uint16_t n_words;
	uint8_t byte_count;
	bool valid_address_range;
	static uint8_t exception_code;

	for(drv_modbus_inst i = 0; i < DRV_MODBUS_INST_MAX; i++)
	{
		if(vdrv_modbus_status[i] != STATUS_STARTED)

			continue;

		switch(vdrv_modbus_state[i])
		{

		case DRV_MODBUS_STATE_IDLE:

			/* Discard every incoming byte until it matches the address */

			if(
				hal_uart_retrieve(drv_modbus_uart_inst[i],
								  drv_modbus_frame_buffer[i],
								  1)
				  == ERROR_NONE
				&&
				drv_modbus_frame_buffer[i][0] == vdrv_modbus_addr[i]
			  )
			{
				/* The received byte matches the device address */

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_RECEIVING;

				/* The first byte of the frame is already occupied by the device
				 * address */
				drv_modbus_frame_index[i] = 1;

				/* The timeout is what delimits a frame */
				hal_timer_attach(drv_modbus_timer_inst[i],
								 &vdrv_modbus_timer[i],
								 DRV_MODBUS_TIMEOUT_BETWEEN_BYTES_MS);
			}

			break;

		case DRV_MODBUS_STATE_RECEIVING:

			if(hal_uart_retrieve(drv_modbus_uart_inst[i],
								 &drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]],
								 1)
				  == ERROR_NONE)
			{
				if(++drv_modbus_frame_index[i] >= DRV_MODBUS_MAX_FRAME_LEN_BYTES)

					/* Too many bytes are being received */
					vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

				else

					hal_timer_attach(drv_modbus_timer_inst[i],
									 &vdrv_modbus_timer[i],
									 DRV_MODBUS_TIMEOUT_BETWEEN_BYTES_MS);

			}
			else if(hal_timer_status_get(&vdrv_modbus_timer[i])
					!= HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED)
			{
				vdrv_modbus_state[i] = DRV_MODBUS_STATE_CHECK_CRC;
			}

			break;

		case DRV_MODBUS_STATE_CHECK_CRC:

			/* First of all, let's validate the CRC. Remember that the 2 last
			 * received bytes contain to the CRC */
			drv_modbus_crc_calc(drv_modbus_frame_buffer[i],
								drv_modbus_frame_index[i] - 2,
								crc);

			if(crc[0] == drv_modbus_frame_buffer[i][drv_modbus_frame_index[i] - 2]
				&&
			   crc[1] == drv_modbus_frame_buffer[i][drv_modbus_frame_index[i] - 1])
			{
				/* CRC match */
				vdrv_modbus_state[i] = DRV_MODBUS_STATE_CHECK_FC;
			}
			else

				/* CRC doesn't match */
				vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			break;

		case DRV_MODBUS_STATE_CHECK_FC:

			/* Byte 1 contains the Function Code */
			if(drv_modbus_frame_buffer[i][1] == DRV_MODBUS_FUNCTION_CODE_READ_HOLDING_REGS)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_READ_HOLDING_REGS;

			else if(drv_modbus_frame_buffer[i][1] == DRV_MODBUS_FUNCTION_CODE_READ_INPUT_REGS)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_READ_INPUT_REGS;

			else if(drv_modbus_frame_buffer[i][1] == DRV_MODBUS_FUNCTION_CODE_WRITE_SINGLE_REG)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_WRITE_SINGLE_REG;

			else if(drv_modbus_frame_buffer[i][1] == DRV_MODBUS_FUNCTION_CODE_WRITE_MULTIPLE_REGS)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_WRITE_MULTIPLE_REGS;

			else
			{
				/* Unknown Function Code. Build exception response */

				exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_FUNCTION;

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
			}

			break;

		case DRV_MODBUS_STATE_READ_HOLDING_REGS:

			/* We know that the whole request frame must be exactly 8 bytes
			 * long. If it is not, then no response must be sent, and the frame
			 * must be ignored */

			if(drv_modbus_frame_index[i] != 8)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			else
			{
				/* The requested address is contained in bytes 2 and 3 */

				requested_address =
						(uint16_t)drv_modbus_frame_buffer[i][2] << 8
						| drv_modbus_frame_buffer[i][3];

				/* Check if the requested address is implemented (i.e. if the
				 * register exists) */

				for(reg_index = 0;
					reg_index < vdrv_modbus_regs[i].num_holding_regs;
					reg_index++)
				{
					if(vdrv_modbus_regs[i].holding_regs_addr[reg_index]
						 == requested_address)

						/* Address found, meaning the client requested a valid
						 * address */
						break;
				}

				/* The number of registers is contained in bytes 4 and 5 */

				n_words =
						(uint16_t)drv_modbus_frame_buffer[i][4] << 8
						| drv_modbus_frame_buffer[i][5];

				/* Check if the request leads to an inexistent address */

				valid_address_range = true;

				if(reg_index >= vdrv_modbus_regs[i].num_holding_regs)

					/* The start address is not implemented */
					valid_address_range = false;

				/* Registers must be contiguous */
				for(j = reg_index + 1; j < reg_index + n_words; j++)

					if(
					   (vdrv_modbus_regs[i].holding_regs_addr[j]
							  != vdrv_modbus_regs[i].holding_regs_addr[j - 1] + 1)
						||
						(j >= DRV_MODBUS_0_HOLDING_REG_MAX)
					   )

						/* The request leads to an unimplemented address */
						valid_address_range = false;

				/* Time to make a decision */
				if(n_words < 1 || n_words > 125)
				{
					/* According to the protocol, the requested number of words
					 * must be between 1 and 125 (both included). Illegal data
					 * value */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_VALUE;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
				else if(valid_address_range == false)
				{
					/* Illegal address */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_ADDRESS;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
				else
				{
					/* Request OK. Build response */

					/* Bytes 0 and 1 already contain the server address and the
					 * function code respectively, and shall not be modified */

					/* Byte 2 contains the byte count */
					drv_modbus_frame_buffer[i][2] = n_words << 1;

					/* The next bytes contain the register values */

					drv_modbus_frame_index[i] = 3;

					for(j = 0; j < n_words; j++)
					{
						/* High order byte first */
						drv_modbus_frame_buffer[i][drv_modbus_frame_index[i] + (j << 1)]
						   = (uint8_t)(vdrv_modbus_regs[i].holding_regs_val[reg_index + j] >> 8 & 0x00FF);

						/* Low order byte */
						drv_modbus_frame_buffer[i][drv_modbus_frame_index[i] + (j << 1) + 1]
						   = (uint8_t)(vdrv_modbus_regs[i].holding_regs_val[reg_index + j] & 0x00FF);
					}

					drv_modbus_frame_index[i] += n_words << 1;

					/* CRC */

					drv_modbus_crc_calc(drv_modbus_frame_buffer[i],
										drv_modbus_frame_index[i],
										crc);

					drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]++]
					   = crc[0];

					drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]++]
					   = crc[1];

					/* Delay before sending response */
					hal_timer_attach(drv_modbus_timer_inst[i],
									 &vdrv_modbus_timer[i],
									 DRV_MODBUS_TIME_BETWEEN_FRAMES_MS);


					vdrv_modbus_state[i] = DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE;
				}
			}

			break;

		case DRV_MODBUS_STATE_READ_INPUT_REGS:

			/* We know that the whole request frame must be exactly 8 bytes
			 * long. If it is not, then no response must be sent, and the frame
			 * must be ignored */

			if(drv_modbus_frame_index[i] != 8)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			else
			{
				/* The requested address is contained in bytes 2 and 3 */

				requested_address =
						(uint16_t)drv_modbus_frame_buffer[i][2] << 8
						| drv_modbus_frame_buffer[i][3];

				/* Check if the requested address is implemented (i.e. if the
				 * register exists) */

				for(reg_index = 0;
					reg_index < vdrv_modbus_regs[i].num_input_regs;
					reg_index++)
				{
					if(vdrv_modbus_regs[i].input_regs_addr[reg_index]
						 == requested_address)

						/* Address found, meaning the client requested a valid
						 * address */
						break;
				}

				/* The number of registers is contained in bytes 4 and 5 */

				n_words =
						(uint16_t)drv_modbus_frame_buffer[i][4] << 8
						| drv_modbus_frame_buffer[i][5];

				/* Check if the request leads to an inexistent address */

				valid_address_range = true;

				if(reg_index >= vdrv_modbus_regs[i].num_input_regs)

					/* The start address is not implemented */
					valid_address_range = false;

				/* Registers must be contiguous */
				for(j = reg_index + 1; j < reg_index + n_words; j++)

					if(
					   (vdrv_modbus_regs[i].input_regs_addr[j]
							  != vdrv_modbus_regs[i].input_regs_addr[j - 1] + 1)
						||
						(j >= DRV_MODBUS_0_INPUT_REG_MAX)
					   )

						/* The request leads to an unimplemented address */
						valid_address_range = false;

				/* Time to make a decision */
				if(n_words < 1 || n_words > 125)
				{
					/* According to the protocol, the requested number of words
					 * must be between 1 and 125 (both included). Illegal data
					 * value */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_VALUE;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
				else if(valid_address_range == false)
				{
					/* Illegal address */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_ADDRESS;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
				else
				{
					/* Request OK. Build response */

					/* Bytes 0 and 1 already contain the server address and the
					 * function code respectively, and shall not be modified */

					/* Byte 2 contains the byte count */
					drv_modbus_frame_buffer[i][2] = n_words << 1;

					/* The next bytes contain the register values */

					drv_modbus_frame_index[i] = 3;

					for(j = 0; j < n_words; j++)
					{
						/* High order byte first */
						drv_modbus_frame_buffer[i][drv_modbus_frame_index[i] + (j << 1)]
						   = (uint8_t)(vdrv_modbus_regs[i].input_regs_val[reg_index + j] >> 8 & 0x00FF);

						/* Low order byte */
						drv_modbus_frame_buffer[i][drv_modbus_frame_index[i] + (j << 1) + 1]
						   = (uint8_t)(vdrv_modbus_regs[i].input_regs_val[reg_index + j] & 0x00FF);
					}

					drv_modbus_frame_index[i] += n_words << 1;

					/* CRC */

					drv_modbus_crc_calc(drv_modbus_frame_buffer[i],
										drv_modbus_frame_index[i],
										crc);

					drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]++]
					   = crc[0];

					drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]++]
					   = crc[1];

					/* Delay before sending response */
					hal_timer_attach(drv_modbus_timer_inst[i],
									 &vdrv_modbus_timer[i],
									 DRV_MODBUS_TIME_BETWEEN_FRAMES_MS);

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE;
				}
			}

			break;

		case DRV_MODBUS_STATE_WRITE_SINGLE_REG:

			/* We know that the whole request frame must be exactly 8 bytes
			 * long. If it is not, then no response must be sent, and the frame
			 * must be ignored */

			if(drv_modbus_frame_index[i] != 8)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			else
			{
				/* The requested address is contained in bytes 2 and 3 */

				requested_address =
						(uint16_t)drv_modbus_frame_buffer[i][2] << 8
						| drv_modbus_frame_buffer[i][3];

				/* Check if the requested address is implemented (i.e. if the
				 * register exists) */

				for(reg_index = 0;
					reg_index < vdrv_modbus_regs[i].num_holding_regs;
					reg_index++)
				{
					if(vdrv_modbus_regs[i].holding_regs_addr[reg_index]
						 == requested_address)

						/* Address found, meaning the client requested a valid
						 * address */
						break;
				}

				/* If the requested register has been found, then the request
				 * can be processed */
				if(reg_index < vdrv_modbus_regs[i].num_holding_regs)
				{
					/* Register found */

					/* Bytes 4 and 5 contain the register value */

					vdrv_modbus_regs[i].holding_regs_val[reg_index]
						  = (uint16_t)(drv_modbus_frame_buffer[i][4]) << 8
							| drv_modbus_frame_buffer[i][5];

					/* The response is exactly the same as the request, so no
					 * need to modify drv_modbus_frame_buffer[i] */

					/* Delay before sending response */
					hal_timer_attach(drv_modbus_timer_inst[i],
									 &vdrv_modbus_timer[i],
									 DRV_MODBUS_TIME_BETWEEN_FRAMES_MS);

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE;
				}
				else
				{
					/* Illegal address */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_ADDRESS;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
			}

			break;

		case DRV_MODBUS_STATE_WRITE_MULTIPLE_REGS:

			/* Quantity of registers is specified in bytes 4 and 5 */
			n_words =
					(uint16_t)drv_modbus_frame_buffer[i][4] << 8
					| drv_modbus_frame_buffer[i][5];

			/* Byte count is specified in byte 6 */
			byte_count = drv_modbus_frame_buffer[i][6];

			/* Knowing the quantity of registers, the correct frame length can
			 * be calculated. If the frame exceeds the length or lacks bytes,
			 * then it must be ignored */

			if(drv_modbus_frame_index[i] != 9 + byte_count)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			else
			{
				/* The requested address is contained in bytes 2 and 3 */

				requested_address =
						(uint16_t)drv_modbus_frame_buffer[i][2] << 8
						| drv_modbus_frame_buffer[i][3];

				/* Check if the requested address is implemented (i.e. if the
				 * register exists) */

				for(reg_index = 0;
					reg_index < vdrv_modbus_regs[i].num_holding_regs;
					reg_index++)
				{
					if(vdrv_modbus_regs[i].holding_regs_addr[reg_index]
						 == requested_address)

						/* Address found, meaning the client requested a valid
						 * address */
						break;
				}

				/* Check if the request leads to an inexistent address */

				valid_address_range = true;

				if(reg_index >= vdrv_modbus_regs[i].num_holding_regs)

					/* The start address is not implemented */
					valid_address_range = false;

				/* Registers must be contiguous */
				for(j = reg_index + 1; j < reg_index + n_words; j++)

					if(
					   (vdrv_modbus_regs[i].holding_regs_addr[j]
							  != vdrv_modbus_regs[i].holding_regs_addr[j - 1] + 1)
						||
						(j >= DRV_MODBUS_0_HOLDING_REG_MAX)
					   )

						/* The request leads to an unimplemented address */
						valid_address_range = false;

				/* Time to make a decision */
				if(n_words < 1 || n_words > 123)
				{
					/* According to the protocol, the requested number of words
					 * must be between 1 and 123 (both included). Illegal data
					 * value */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_VALUE;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
				else if(valid_address_range == false)
				{
					/* Illegal address */

					exception_code = DRV_MODBUS_EXCEPTION_CODE_ILLEGAL_DATA_ADDRESS;

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE;
				}
				else
				{
					/* Request OK */

					/* Perform the write */

					for(j = reg_index; j < reg_index + n_words; j++)
					{
						vdrv_modbus_regs[i].holding_regs_val[j]
							 = (uint16_t)drv_modbus_frame_buffer[i][7 + ((j - reg_index) << 1)]
								|  drv_modbus_frame_buffer[i][7 + ((j - reg_index) << 1) + 1];
					}

					/* Build response */

					/* The first 6 bytes of the response are the first 6 bytes
					 * of the request */

					drv_modbus_frame_index[i] = 6;

					/* CRC */

					drv_modbus_crc_calc(drv_modbus_frame_buffer[i],
										drv_modbus_frame_index[i],
										crc);

					drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]++]
					   = crc[0];

					drv_modbus_frame_buffer[i][drv_modbus_frame_index[i]++]
					   = crc[1];

					/* Delay before sending response */
					hal_timer_attach(drv_modbus_timer_inst[i],
									 &vdrv_modbus_timer[i],
									 DRV_MODBUS_TIME_BETWEEN_FRAMES_MS);

					vdrv_modbus_state[i] = DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE;
				}
			}

			break;

		case DRV_MODBUS_STATE_BUILD_EXCEPTION_RESPONSE:

			/* Byte 0 already contains the device address. Byte 1 needs to
			 * be OR'ed with 0x80 */
			drv_modbus_frame_buffer[i][1] |= 0x80;

			/* Byte 2 must contain the exception code */
			drv_modbus_frame_buffer[i][2] = exception_code;

			/* Bytes 3 and 4 must contain the CRC */
			drv_modbus_crc_calc(drv_modbus_frame_buffer[i],
								3,
								&drv_modbus_frame_buffer[i][3]);

			/* Set drv_modbus_frame_index to the total number of bytes to send */
			drv_modbus_frame_index[i] = 5;

			/* Delay before sending response */
			hal_timer_attach(drv_modbus_timer_inst[i],
							 &vdrv_modbus_timer[i],
							 DRV_MODBUS_TIME_BETWEEN_FRAMES_MS);

			/* Exception response built. Send it */
			vdrv_modbus_state[i] = DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE;

			break;

		case DRV_MODBUS_STATE_DELAY_BEFORE_RESPONSE:

			if(hal_timer_status_get(&vdrv_modbus_timer[i])
				!= HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED)

				/* Timer expired. Send response */
				vdrv_modbus_state[i] = DRV_MODBUS_STATE_SEND_RESPONSE;

			break;

		case DRV_MODBUS_STATE_SEND_RESPONSE:

			if(hal_uart_send(drv_modbus_uart_inst[i],
							 drv_modbus_frame_buffer[i],
							 drv_modbus_frame_index[i])
				== ERROR_NONE)

				vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			break;

		default:

			vdrv_modbus_state[i] = DRV_MODBUS_STATE_IDLE;

			break;
		}
	}
}

static void drv_modbus_crc_calc(uint8_t *buff, uint16_t len, uint8_t crc_buff[2])
{
	const uint16_t poly = 0xA001;
	uint16_t crc = 0xFFFF;

	for(uint16_t i = 0; i < len; i++)
	{
		crc ^= buff[i];

		for(uint8_t i = 0; i < 8; i++)
		{
			if((crc & 1U) == 0)
			{
				crc >>= 1;
			}
			else
			{
				crc >>= 1;

				crc ^= poly;
			}
		}
	}

	/* CRC low */
	crc_buff[0] = (uint8_t)(crc & 0x00FFU);

	/* CRC high */
	crc_buff[1] = (uint8_t)((crc & 0xFF00U) >> 8U);
}
