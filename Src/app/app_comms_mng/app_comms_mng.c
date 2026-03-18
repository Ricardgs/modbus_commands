/*
 * app_comms_mng.c
 *
 *  Created on: Feb 19, 2026
 *      Author: ricard
 */

#include "app_comms_mng.h"
#include "drv_modbus/drv_modbus_registers.h"
#include "drv_led/drv_led.h"
#include "drv_push_button/drv_push_button.h"
#include "drv_clk_mng/drv_clk_mng.h"
#include "hal_clk/hal_clk.h"

#define APP_COMMS_MNG_LED_OFF_REG_VAL	0
#define APP_COMMS_MNG_LED_ON_REG_VAL	1
#define APP_COMMS_MNG_LED_BLINK_REG_VAL	2

static uint32_t vapp_comms_mng_baudrate_request[DRV_MODBUS_INST_MAX];

void app_comms_mng_init(void)
{
	for(drv_modbus_inst i = 0; i < DRV_MODBUS_INST_MAX; i++)

		vapp_comms_mng_baudrate_request[i] = DRV_MODBUS_DEFAULT_BAUDRATE;
}

void app_comms_mng_start(void)
{
	uint16_t data_16;

	/* Set default values */

	/* Clock frequency high (input register) */

	data_16 = (uint16_t)(HAL_CLK_TARGET_FREQ_HZ >> 16);

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_INPUT,
							  DRV_MODBUS_0_INPUT_REG_ACTUAL_CLK_FREQ_HIGH,
							  data_16);

	/* Clock frequency low (input register) */

	data_16 = (uint16_t)HAL_CLK_TARGET_FREQ_HZ;

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_INPUT,
							  DRV_MODBUS_0_INPUT_REG_ACTUAL_CLK_FREQ_LOW,
							  data_16);

	/* Clock frequency high (holding register) */

	data_16 = (uint16_t)(HAL_CLK_TARGET_FREQ_HZ >> 16);

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_HOLDING,
							  DRV_MODBUS_0_HOLDING_REG_REQUESTED_CLK_FREQ_HIGH,
							  data_16);

	/* Clock frequency low (holding register) */

	data_16 = (uint16_t)HAL_CLK_TARGET_FREQ_HZ;

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_HOLDING,
							  DRV_MODBUS_0_HOLDING_REG_REQUESTED_CLK_FREQ_LOW,
							  data_16);

	/* Baud rate high */

	data_16 = (uint16_t)(DRV_MODBUS_DEFAULT_BAUDRATE >> 16);

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_HOLDING,
							  DRV_MODBUS_0_HOLDING_REG_BAUDRATE_HIGH,
							  data_16);

	/* Baud rate low */

	data_16 = (uint16_t)DRV_MODBUS_DEFAULT_BAUDRATE;

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_HOLDING,
							  DRV_MODBUS_0_HOLDING_REG_BAUDRATE_LOW,
							  data_16);
}

void app_comms_mng_fxn(void)
{
	uint16_t data_16;
	uint32_t data_32;

	/* Modbus 0 input registers */

	/* Push button */

	data_16 = (uint16_t)(drv_push_button_read(DRV_PUSH_BUTTON_0));

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_INPUT,
							  DRV_MODBUS_0_INPUT_REG_PUSH_BUTTON,
							  data_16);

	/* Actual clock frequency */

	data_32 = hal_clk_get_freq_hz();

	data_16 = (uint16_t)(data_32 >> 16);

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_INPUT,
							  DRV_MODBUS_0_INPUT_REG_ACTUAL_CLK_FREQ_HIGH,
							  data_16);

	data_16 = (uint16_t)data_32;

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_INPUT,
							  DRV_MODBUS_0_INPUT_REG_ACTUAL_CLK_FREQ_LOW,
							  data_16);

	/* Modbus 0 holding registers */

	/* Clock frequency */

	drv_modbus_read_register(DRV_MODBUS_INST_0,
							 DRV_MODBUS_REGISTER_TYPE_HOLDING,
							 DRV_MODBUS_0_HOLDING_REG_REQUESTED_CLK_FREQ_HIGH,
							 &data_16);

	data_32 = ((uint32_t)data_16) << 16;

	drv_modbus_read_register(DRV_MODBUS_INST_0,
							 DRV_MODBUS_REGISTER_TYPE_HOLDING,
							 DRV_MODBUS_0_HOLDING_REG_REQUESTED_CLK_FREQ_LOW,
							 &data_16);

	data_32 |= (uint32_t)data_16;

	drv_clk_mng_set_request(data_32);

	/* Baud rate */

	drv_modbus_read_register(DRV_MODBUS_INST_0,
							 DRV_MODBUS_REGISTER_TYPE_HOLDING,
							 DRV_MODBUS_0_HOLDING_REG_BAUDRATE_HIGH,
							 &data_16);

	data_32 = ((uint32_t)data_16) << 16;

	drv_modbus_read_register(DRV_MODBUS_INST_0,
							 DRV_MODBUS_REGISTER_TYPE_HOLDING,
							 DRV_MODBUS_0_HOLDING_REG_BAUDRATE_LOW,
							 &data_16);

	data_32 |= (uint32_t)data_16;

	vapp_comms_mng_baudrate_request[DRV_MODBUS_INST_0] = data_32;

	/* Led */

	drv_modbus_read_register(DRV_MODBUS_INST_0,
							 DRV_MODBUS_REGISTER_TYPE_HOLDING,
							 DRV_MODBUS_0_HOLDING_REG_LED,
							 &data_16);

	if(data_16 == 0)

		drv_led_set_request(DRV_LED_INST_0, DRV_LED_REQUEST_OFF);

	else if(data_16 == 1)

		drv_led_set_request(DRV_LED_INST_0, DRV_LED_REQUEST_ON);

	else

		drv_led_set_request(DRV_LED_INST_0, DRV_LED_REQUEST_BLINK);

}

uint32_t app_comms_mng_baurdate_request_get(drv_modbus_inst modbus_inst)
{
	if(modbus_inst < DRV_MODBUS_INST_MAX)

		return vapp_comms_mng_baudrate_request[modbus_inst];

	else

		return 0;
}

