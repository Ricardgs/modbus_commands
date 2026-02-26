/*
 * hal_pin_mat.c
 *
 *  Created on: Jan 17, 2026
 *      Author: ricard
 */


#include "hal_pin_mat.h"

static void hal_pin_mat_enable_port_clk(const GPIO_TypeDef *port);

extern const hal_pin_mat_pin_type_s chal_pin_mat_pin_matrix[HAL_MAT_PIN_MAX];

void hal_pin_mat_init(void)
{
	/* Nothing to be done */
}

void hal_pin_mat_start(void)
{
	hal_mat_pin_e pin;

	for(pin = 0; pin < HAL_MAT_PIN_MAX; pin++)
	{
		/* Enable port clock */
		hal_pin_mat_enable_port_clk(chal_pin_mat_pin_matrix[pin].port);

		/* MODER */
		chal_pin_mat_pin_matrix[pin].port->MODER
			&= ~(3UL << (chal_pin_mat_pin_matrix[pin].pin * 2));

		chal_pin_mat_pin_matrix[pin].port->MODER
			|= ((uint32_t)chal_pin_mat_pin_matrix[pin].mode) << (chal_pin_mat_pin_matrix[pin].pin * 2);

		/* OTYPER */
		chal_pin_mat_pin_matrix[pin].port->OTYPER
			&= ~(1UL << chal_pin_mat_pin_matrix[pin].pin);

		chal_pin_mat_pin_matrix[pin].port->OTYPER
			|= ((uint32_t)chal_pin_mat_pin_matrix[pin].output_type) << chal_pin_mat_pin_matrix[pin].pin;

		/* OSPEEDR */
		chal_pin_mat_pin_matrix[pin].port->OSPEEDR
			&= ~(3UL << (chal_pin_mat_pin_matrix[pin].pin * 2));

		chal_pin_mat_pin_matrix[pin].port->OSPEEDR
			|= ((uint32_t)chal_pin_mat_pin_matrix[pin].output_speed) << (chal_pin_mat_pin_matrix[pin].pin * 2);

		/* PUPDR */
		chal_pin_mat_pin_matrix[pin].port->PUPDR
			&= ~(3UL << (chal_pin_mat_pin_matrix[pin].pin * 2));

		chal_pin_mat_pin_matrix[pin].port->PUPDR
			|= ((uint32_t)chal_pin_mat_pin_matrix[pin].pup_pdown) << (chal_pin_mat_pin_matrix[pin].pin * 2);

		if(chal_pin_mat_pin_matrix[pin].pin < 8)
		{
			/* AFR low */
			chal_pin_mat_pin_matrix[pin].port->AFR[0]
				&= ~(15UL << (chal_pin_mat_pin_matrix[pin].pin * 4));

			chal_pin_mat_pin_matrix[pin].port->AFR[0]
				|= ((uint32_t)chal_pin_mat_pin_matrix[pin].alter_func_low_sel) << (chal_pin_mat_pin_matrix[pin].pin * 4);
		}
		else
		{
			/* AFR high */
			chal_pin_mat_pin_matrix[pin].port->AFR[1]
				&= ~(15UL << ((chal_pin_mat_pin_matrix[pin].pin - 8) * 4));

			chal_pin_mat_pin_matrix[pin].port->AFR[1]
				|= ((uint32_t)chal_pin_mat_pin_matrix[pin].alter_func_high_sel) << ((chal_pin_mat_pin_matrix[pin].pin - 8) * 4);
		}
	}
}

const hal_pin_mat_pin_type_s chal_pin_mat_pin_matrix[HAL_MAT_PIN_MAX] =
{
		/* HAL_MAT_PIN_USER_LED */
		{ GPIOA,	// port					-> A
		  5,		// pin					-> 5
		  1,		// mode					-> GPO
		  0,		// output_type			-> push-pull
		  0,		// output_speed			-> low
		  0,		// pup_pdown			-> none
		  0,		// alter_func_low_sel	-> not used
		  0			// alter_func_high_sel	-> not used
		},

		/* HAL_MAT_PIN_PUSH_BUTTON */
		{ GPIOC,	// port					-> C
		  13,		// pin					-> 13
		  0,		// mode					-> input
		  0,		// output_type			-> not used
		  0,		// output_speed			-> not used
		  0,		// pup_pdown			-> none
		  0,		// alter_func_low_sel	-> not used
		  0			// alter_func_high_sel	-> not used
		},

		/* HAL_MAT_PIN_USART2_TX */
		{ GPIOA,	// port					-> A
		  2,		// pin					-> 2
		  2,		// mode					-> alternate
		  0,		// output_type			-> push-pull
		  0,		// output_speed			-> low
		  0,		// pup_pdown			-> none
		  7,		// alter_func_low_sel	-> AF7
		  0			// alter_func_high_sel	-> not used
		},

		/* HAL_MAT_PIN_USART2_RX */
		{ GPIOA,	// port					-> A
		  3,		// pin					-> 3
		  2,		// mode					-> alternate
		  0,		// output_type			-> not used
		  0,		// output_speed			-> not used
		  0,		// pup_pdown			-> none
		  7,		// alter_func_low_sel	-> AF7
		  0			// alter_func_high_sel	-> not used
		},
};

static void hal_pin_mat_enable_port_clk(const GPIO_TypeDef *port)
{
	if(port == GPIOA)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;

	else if(port == GPIOB)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk;

	else if(port == GPIOC)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN_Msk;

	else if(port == GPIOD)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN_Msk;

	else if(port == GPIOE)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN_Msk;

	else if(port == GPIOF)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOFEN_Msk;

	else if(port == GPIOG)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN_Msk;

	else if(port == GPIOH)

		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOHEN_Msk;

}
