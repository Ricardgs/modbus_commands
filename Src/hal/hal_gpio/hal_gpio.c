/*
 * hal_gpio.c
 *
 *  Created on: Jan 18, 2026
 *      Author: ricard
 */


#include "hal_gpio.h"

static GPIO_TypeDef *hal_gpio_get_type(hal_gpio_inst_s inst);

void hal_gpio_init(void)
{
	/* Nothing to be done */
}

void hal_gpio_start(void)
{
	/* Nothing to be done */
}

uint8_t hal_gpio_pin_read(hal_gpio_inst_s inst, uint8_t pin)
{
	return (hal_gpio_get_type(inst)->IDR >> pin) & 1UL;
}

void hal_gpio_pin_set(hal_gpio_inst_s inst, uint8_t pin)
{
	hal_gpio_get_type(inst)->ODR |= 1UL << pin;
}

void hal_gpio_pin_clear(hal_gpio_inst_s inst, uint8_t pin)
{
	hal_gpio_get_type(inst)->ODR &= ~(1UL << pin);
}

void hal_gpio_pin_toggle(hal_gpio_inst_s inst, uint8_t pin)
{
	hal_gpio_get_type(inst)->ODR ^= 1UL << pin;
}


static GPIO_TypeDef *hal_gpio_get_type(hal_gpio_inst_s inst)
{
	if(inst == HAL_GPIO_INST_A)

		return GPIOA;

	else if(inst == HAL_GPIO_INST_B)

		return GPIOB;

	else if(inst == HAL_GPIO_INST_B)

		return GPIOB;

	else if(inst == HAL_GPIO_INST_C)

		return GPIOC;

	else if(inst == HAL_GPIO_INST_D)

		return GPIOD;

	else if(inst == HAL_GPIO_INST_E)

		return GPIOE;

	else if(inst == HAL_GPIO_INST_F)

		return GPIOF;

	else

		return GPIOG;
}
