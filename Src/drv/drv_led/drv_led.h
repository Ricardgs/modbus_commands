/*
 * drv_led.h
 *
 *  Created on: Jan 19, 2026
 *      Author: ricard
 */

#ifndef DRV_DRV_LED_DRV_LED_H_
#define DRV_DRV_LED_DRV_LED_H_

#include "hal_gpio/hal_gpio.h"
#include "hal_timer/hal_timer.h"

typedef enum
{
	DRV_LED_INST_0,
	DRV_LED_INST_MAX
} drv_led_inst_e;

typedef struct
{
	drv_led_inst_e led_inst;
	hal_gpio_inst_s gpio_inst;
	uint8_t pin;
	hal_timer_timer_inst_e timer_inst;
} drv_led_congif_s;

typedef enum
{
	DRV_LED_REQUEST_OFF,
	DRV_LED_REQUEST_ON,
	DRV_LED_REQUEST_BLINK,
	DRV_LED_REQUEST_MAX
} drv_led_request_e;

void drv_led_init(void);
void drv_led_start(const drv_led_congif_s led_config);
void drv_led_fxn(void);
void drv_led_set_request(drv_led_inst_e led_inst, drv_led_request_e request);

#endif /* DRV_DRV_LED_DRV_LED_H_ */
