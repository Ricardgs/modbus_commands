/*
 * hal_pin_mat.h
 *
 *  Created on: Jan 17, 2026
 *      Author: ricard
 */

#ifndef HAL_HAL_PIN_MAT_HAL_PIN_MAT_H_
#define HAL_HAL_PIN_MAT_HAL_PIN_MAT_H_

#include <stm32l476xx.h>

typedef struct
{
	GPIO_TypeDef *port;
	uint32_t pin					: 4;
	uint32_t mode					: 2;
	uint32_t output_type			: 1;
	uint32_t output_speed			: 2;
	uint32_t pup_pdown				: 2;
	uint32_t alter_func_low_sel		: 4;
	uint32_t alter_func_high_sel	: 4;
} hal_pin_mat_pin_type_s;

typedef enum
{
	HAL_MAT_PIN_USER_LED,
	HAL_MAT_PIN_PUSH_BUTTON,
	HAL_MAT_PIN_USART2_TX,
	HAL_MAT_PIN_USART2_RX,
	HAL_MAT_PIN_MAX,
} hal_mat_pin_e;

void hal_pin_mat_init(void);
void hal_pin_mat_start(void);

#endif /* HAL_HAL_PIN_MAT_HAL_PIN_MAT_H_ */
