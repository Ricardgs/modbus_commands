/*
 * drv_push_button.c
 *
 *  Created on: Jan 19, 2026
 *      Author: ricard
 */


#include "drv_push_button.h"

static drv_hal_push_button_config_s vdrv_push_button_type[DRV_PUSH_BUTTON_MAX];

void drv_push_button_init(void)
{
	/* Nothing to be done */
}

void drv_push_button_start(const drv_hal_push_button_config_s push_button_config)
{
	if(push_button_config.push_button_inst < DRV_PUSH_BUTTON_MAX)
	{
		vdrv_push_button_type[push_button_config.push_button_inst].gpio_inst
			= push_button_config.gpio_inst;

		vdrv_push_button_type[push_button_config.push_button_inst].pin
			= push_button_config.pin;
	}
}

void drv_push_button_fxn(void)
{
	//TODO: anti-bouncing?
}

uint8_t drv_push_button_read(drv_push_button_inst_e push_button_inst)
{
	return hal_gpio_pin_read(vdrv_push_button_type[push_button_inst].gpio_inst,
							 vdrv_push_button_type[push_button_inst].pin);
}
