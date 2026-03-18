/*
 * app_baudrate_mng.c
 *
 *  Created on: Mar 18, 2026
 *      Author: ricard
 */

#include "app_baudrate_mng.h"
#include "drv_modbus/drv_modbus.h"
#include "hal_clk/hal_clk.h"

#define APP_BAUDRATE_MNG_MINIMUM_BAUDRATE	9600
#define APP_BAUDRATE_MNG_MAXIMUM_BAUDRATE	115200

typedef enum
{
	APP_BAUDRATE_MNG_STATE_IDLE,
	APP_BAUDRATE_MNG_STATE_WAIT_MODBUS,
	APP_BAUDRATE_MNG_STATE_DISABLE_UART,
} app_baudrate_mng_state_e;

static uint32_t vapp_baudrate_mng_baudrate_request[DRV_MODBUS_INST_MAX];
static hal_uart_uart_num_e vapp_baudrate_mng_uart_num[DRV_MODBUS_INST_MAX];
static app_baudrate_mng_state_e vapp_baudrate_mng_state[DRV_MODBUS_INST_MAX];

void app_baudrate_mng_init(void)
{
	/* To protect against uninitialized values, initialize to an incorrect
	 * value */

	for(drv_modbus_inst i = 0; i < DRV_MODBUS_INST_MAX; i++)
	{
		/* With this value, all baud rate change requests will be ignored */
		vapp_baudrate_mng_uart_num[i] = HAL_UART_UART_MAX;

		vapp_baudrate_mng_state[i] = APP_BAUDRATE_MNG_STATE_IDLE;

		vapp_baudrate_mng_baudrate_request[i] = 0;
	}
}

void app_baudrate_mng_start(const app_baudrate_mng_config_t *config)
{
	if(config->modbus_inst < DRV_MODBUS_INST_MAX
		&& config->uart_num < HAL_UART_UART_MAX)
	{
		/* Now the variable vapp_baudrate_mng_uart_num will have a valid value,
		 * at least in the initialized position. Baud rate change requests will
		 * be accepted */
		vapp_baudrate_mng_uart_num[config->modbus_inst] = config->uart_num;

		vapp_baudrate_mng_baudrate_request[config->modbus_inst] = config->baudrate;
	}
}

void app_baudrate_mng_fxn(void)
{
	for(drv_modbus_inst i = 0; i < DRV_MODBUS_INST_MAX; i++)
	{
		if(vapp_baudrate_mng_uart_num[i] >= HAL_UART_UART_MAX)

			continue;

		switch(vapp_baudrate_mng_state[i])
		{

		case APP_BAUDRATE_MNG_STATE_IDLE:

			if(vapp_baudrate_mng_baudrate_request[i]
			  != hal_uart_baudrate_get(vapp_baudrate_mng_uart_num[i]))

				/* A new baud rate has been requested */

				vapp_baudrate_mng_state[i]
					= APP_BAUDRATE_MNG_STATE_WAIT_MODBUS;

			break;

		case APP_BAUDRATE_MNG_STATE_WAIT_MODBUS:

			/* The response needs to have been sent before modifying baud rate */

			if(drv_modbus_status_get(i) == DRV_MODBUS_STATUS_IDLE)
			{
				vapp_baudrate_mng_state[i]
					= APP_BAUDRATE_MNG_STATE_DISABLE_UART;
			}

			break;

		case APP_BAUDRATE_MNG_STATE_DISABLE_UART:

			hal_uart_disable(vapp_baudrate_mng_uart_num[i]);

			if(hal_uart_is_disabled(vapp_baudrate_mng_uart_num[i]) == true)
			{
				/* Proceed to change baud rate */

				hal_uart_change_baudrate(vapp_baudrate_mng_uart_num[i],
										 vapp_baudrate_mng_baudrate_request[i],
										 hal_clk_get_freq_hz());

				/* Re-enable uart */
				hal_uart_enable(vapp_baudrate_mng_uart_num[i]);

				vapp_baudrate_mng_state[i]
					= APP_BAUDRATE_MNG_STATE_IDLE;
			}

			break;

		default:

			/* Should never reach here */

			break;
		}
	}
}

void app_baudrate_new_baudrate_set(drv_modbus_inst modbus_inst,
		   	   	   	   	   	   	   uint32_t baudrate)
{
	if(modbus_inst < DRV_MODBUS_INST_MAX
		&& baudrate >= APP_BAUDRATE_MNG_MINIMUM_BAUDRATE
		&& baudrate <= APP_BAUDRATE_MNG_MAXIMUM_BAUDRATE)

		vapp_baudrate_mng_baudrate_request[modbus_inst] = baudrate;
}
