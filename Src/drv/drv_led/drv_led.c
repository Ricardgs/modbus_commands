/*
 * drv_led.c
 *
 *  Created on: Jan 19, 2026
 *      Author: ricard
 */


#include "drv_led.h"
#include "hal_timer/hal_timer.h"

#define DRV_LED_BLINK_TIME_MS	1000

typedef enum
{
	DRV_LED_STATE_OFF,
	DRV_LED_STATE_ON,
	DRV_LED_STATE_BLINK,
	DRV_LED_STATE_DECIDE_NEXT_STATE,
	DRV_LED_STATE_MAX
} drv_led_state_e;

static drv_led_congif_s vdrv_led_type[DRV_LED_INST_MAX];
static drv_led_request_e vdrv_led_request[DRV_LED_INST_MAX];
static drv_led_state_e vdrv_led_state[DRV_LED_INST_MAX];
static hal_timer_timer_inst_e vdrv_led_timer_inst[DRV_LED_INST_MAX];
static hal_timer_timer_s vdrv_led_timer[DRV_LED_INST_MAX];

void drv_led_init(void)
{
	for(drv_led_inst_e i = 0; i < DRV_LED_INST_MAX; i++)
	{
		vdrv_led_request[i] = DRV_LED_REQUEST_OFF;
		vdrv_led_state[i] = DRV_LED_STATE_OFF;
		hal_timer_detach(&vdrv_led_timer[i]);
	}
}

void drv_led_start(const drv_led_congif_s led_config)
{
	if(led_config.led_inst < DRV_LED_INST_MAX)
	{
		vdrv_led_type[led_config.led_inst].gpio_inst = led_config.gpio_inst;
		vdrv_led_type[led_config.led_inst].pin = led_config.pin;
		vdrv_led_timer_inst[led_config.led_inst] = led_config.timer_inst;
	}
}

void drv_led_fxn(void)
{
	for(drv_led_inst_e i = 0; i < DRV_LED_INST_MAX; i++)
	{
		switch(vdrv_led_state[i])
		{
		case DRV_LED_STATE_OFF:

			hal_gpio_pin_clear(vdrv_led_type[i].gpio_inst,
							   vdrv_led_type[i].pin);

			if(vdrv_led_request[i] != DRV_LED_REQUEST_OFF)

				vdrv_led_state[i] = DRV_LED_STATE_DECIDE_NEXT_STATE;

			break;

		case DRV_LED_STATE_ON:

			hal_gpio_pin_set(vdrv_led_type[i].gpio_inst,
							 vdrv_led_type[i].pin);

			if(vdrv_led_request[i] != DRV_LED_REQUEST_ON)

				vdrv_led_state[i] = DRV_LED_STATE_DECIDE_NEXT_STATE;

			break;

		case DRV_LED_STATE_BLINK:

			if(hal_timer_status_get(&vdrv_led_timer[i])
				!= HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED)
			{
				hal_gpio_pin_toggle(vdrv_led_type[i].gpio_inst,
								 	vdrv_led_type[i].pin);

				hal_timer_attach(vdrv_led_timer_inst[i],
								 &vdrv_led_timer[i],
								 DRV_LED_BLINK_TIME_MS);
			}

			if(vdrv_led_request[i] != DRV_LED_REQUEST_BLINK)

				vdrv_led_state[i] = DRV_LED_STATE_DECIDE_NEXT_STATE;

			break;

		case DRV_LED_STATE_DECIDE_NEXT_STATE:

			if(vdrv_led_request[i] == DRV_LED_REQUEST_ON)

				vdrv_led_state[i] = DRV_LED_STATE_ON;

			else if(vdrv_led_request[i] == DRV_LED_REQUEST_BLINK)

				vdrv_led_state[i] = DRV_LED_STATE_BLINK;

			else

				vdrv_led_state[i] = DRV_LED_STATE_OFF;

			break;

		default:

			/* Should never reach here */

			vdrv_led_state[i] = DRV_LED_STATE_OFF;

			break;

		}
	}
}

void drv_led_set_request(drv_led_inst_e led_inst, drv_led_request_e request)
{
	vdrv_led_request[led_inst] = request;
}
