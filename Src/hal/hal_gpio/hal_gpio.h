/*
 * hal_gpio.h
 *
 *  Created on: Jan 18, 2026
 *      Author: ricard
 */

#ifndef HAL_HAL_GPIO_HAL_GPIO_H_
#define HAL_HAL_GPIO_HAL_GPIO_H_

#include <stm32l476xx.h>
#include <stdint.h>

typedef enum
{
	HAL_GPIO_INST_A,
	HAL_GPIO_INST_B,
	HAL_GPIO_INST_C,
	HAL_GPIO_INST_D,
	HAL_GPIO_INST_E,
	HAL_GPIO_INST_F,
	HAL_GPIO_INST_G,
	HAL_GPIO_INST_H
} hal_gpio_inst_s;

void hal_gpio_init(void);
void hal_gpio_start(void);
uint8_t hal_gpio_pin_read(hal_gpio_inst_s inst, uint8_t pin);
void hal_gpio_pin_set(hal_gpio_inst_s inst, uint8_t pin);
void hal_gpio_pin_clear(hal_gpio_inst_s inst, uint8_t pin);
void hal_gpio_pin_toggle(hal_gpio_inst_s inst, uint8_t pin);

#endif /* HAL_HAL_GPIO_HAL_GPIO_H_ */
