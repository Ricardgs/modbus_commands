/*
 * drv_push_button.h
 *
 *  Created on: Jan 19, 2026
 *      Author: ricard
 */

#ifndef DRV_DRV_PUSH_BUTTON_DRV_PUSH_BUTTON_H_
#define DRV_DRV_PUSH_BUTTON_DRV_PUSH_BUTTON_H_

#include "../../hal/hal_gpio/hal_gpio.h"

typedef enum
{
	DRV_PUSH_BUTTON_0,
	DRV_PUSH_BUTTON_MAX
} drv_push_button_inst_e;

typedef struct
{
	drv_push_button_inst_e push_button_inst;
	hal_gpio_inst_s gpio_inst;
	uint8_t pin;
} drv_hal_push_button_config_s;

void drv_push_button_init(void);
void drv_push_button_start(const drv_hal_push_button_config_s push_button_config);
void drv_push_button_fxn(void);
uint8_t drv_push_button_read(drv_push_button_inst_e push_button_inst);

#endif /* DRV_DRV_PUSH_BUTTON_DRV_PUSH_BUTTON_H_ */
