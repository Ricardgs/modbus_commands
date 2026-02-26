/*
 * app_comms_mng.c
 *
 *  Created on: Feb 19, 2026
 *      Author: ricard
 */

#include "app_comms_mng.h"
#include "drv_modbus/drv_modbus_registers.h"
#include "drv_modbus/drv_modbus_common.h"
#include "drv_led/drv_led.h"
#include "drv_push_button/drv_push_button.h"

#define APP_COMMS_MNG_LED_OFF_REG_VAL	0
#define APP_COMMS_MNG_LED_ON_REG_VAL	1
#define APP_COMMS_MNG_LED_BLINK_REG_VAL	2

void app_comms_mng_init(void)
{

}

void app_comms_mng_start(void)
{

}

void app_comms_mng_fxn(void)
{
	uint16_t data;

	/* Modbus 0 input registers */

	/* Push button */

	data = (uint16_t)(drv_push_button_read(DRV_PUSH_BUTTON_0));

	drv_modbus_write_register(DRV_MODBUS_INST_0,
							  DRV_MODBUS_REGISTER_TYPE_INPUT,
							  DRV_MODBUS_0_INPUT_REG_PUSH_BUTTON,
							  data);

	/* Modbus 0 holding registers */

	drv_modbus_read_register(DRV_MODBUS_INST_0,
							 DRV_MODBUS_REGISTER_TYPE_HOLDING,
							 DRV_MODBUS_0_HOLDING_REG_LED,
							 &data);

	if(data == 0)

		drv_led_set_request(DRV_LED_INST_0, DRV_LED_REQUEST_OFF);

	else if(data == 1)

		drv_led_set_request(DRV_LED_INST_0, DRV_LED_REQUEST_ON);

	else

		drv_led_set_request(DRV_LED_INST_0, DRV_LED_REQUEST_BLINK);

}

