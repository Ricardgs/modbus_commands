/*
 * hal_timer.c
 *
 *  Created on: Jan 19, 2026
 *      Author: ricard
 */

#include "hal_timer.h"
#include <string.h>
#include <stm32l476xx.h>
#include <core_cm4.h>

static void hal_timer_calc_prescaler_counts(uint32_t clk_freq_hz,
											uint32_t tick_freq_hz,
											hal_timer_timer_inst_e timer_inst,
											uint32_t *prescaler,
											uint32_t *auto_reload);
static void hal_timer_enable_clk(hal_timer_timer_inst_e timer_inst);
static inline void hal_timer_interrupt_handler(hal_timer_timer_inst_e timer_inst);

static uint32_t vhal_timer_ticks[HAL_TIMER_TIMER_INST_MAX];
static uint32_t vhal_timer_tick_freq_hz[HAL_TIMER_TIMER_INST_MAX];

extern TIM_TypeDef *vhal_timer_base[HAL_TIMER_TIMER_INST_MAX];
extern const uint32_t chal_timer_max_count[HAL_TIMER_TIMER_INST_MAX];
extern const IRQn_Type chal_timer_interrupt_source[HAL_TIMER_TIMER_INST_MAX];

void hal_timer_init(void)
{
	memset(vhal_timer_ticks, 0, sizeof(vhal_timer_ticks));
	memset(vhal_timer_tick_freq_hz, 0, sizeof(vhal_timer_tick_freq_hz));
}

void hal_timer_start(hal_timer_config_s config)
{
	TIM_TypeDef *base;
	uint32_t auto_reload;
	uint32_t prescaler;

	if(config.timer_inst < HAL_TIMER_TIMER_INST_MAX)
	{
		base = vhal_timer_base[config.timer_inst];

		vhal_timer_tick_freq_hz[config.timer_inst] = config.tick_freq_hz;

		hal_timer_calc_prescaler_counts(config.clk_freq_hz,
										config.tick_freq_hz,
										config.timer_inst,
										&prescaler,
										&auto_reload);

		/* Enable timer clock */
		hal_timer_enable_clk(config.timer_inst);

		/* Configure prescaler register */
		base->PSC = prescaler;

		/* Configure auto-reload register */
		base->ARR = auto_reload;

		/* Enable interrupt on update event */
		base->DIER |= TIM_DIER_UIE;

		/* Enable counter */
		base->CR1 |= TIM_CR1_CEN;

		/* Enable interrupt in NVIC */
		NVIC_EnableIRQ(chal_timer_interrupt_source[config.timer_inst]);
	}
}

error_e hal_timer_attach(hal_timer_timer_inst_e timer_inst,
					  	 hal_timer_timer_s *timer,
						 uint32_t timeout_ms)
{
	if(timer_inst >= HAL_TIMER_TIMER_INST_MAX)

		return ERROR_NON_EXISTENT_TIMER;

	timer->time_ref = vhal_timer_ticks[timer_inst];

	timer->diff = timeout_ms;

	timer->status = HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED;

	timer->inst = timer_inst;

	return ERROR_NONE;
}

void hal_timer_detach(hal_timer_timer_s *timer)
{
	timer->status = HAL_TIMER_STATUS_NOT_ATTACHED;
}

hal_timer_status_e hal_timer_status_get(hal_timer_timer_s *timer)
{
	if(timer->inst >= HAL_TIMER_TIMER_INST_MAX)

		return HAL_TIMER_STATUS_NOT_ATTACHED;

	if((timer->status == HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED)
		&& (vhal_timer_ticks[timer->inst] >= timer->time_ref)
		&& (vhal_timer_ticks[timer->inst] <= timer->time_ref + timer->diff))

		/* The timer hasn't expired yet */
		return HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED;

	else if((timer->status == HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED)
		&& ((vhal_timer_ticks[timer->inst] < timer->time_ref)
			|| (vhal_timer_ticks[timer->inst] > timer->time_ref + timer->diff)))
	{
		/* First time this function is called and the timer is expired */
		timer->status = HAL_TIMER_STATUS_TIMEOUT_REACHED;
		return timer->status;
	}
	else if(timer->status == HAL_TIMER_STATUS_TIMEOUT_REACHED)
	{
		/* Second time this function is called and the timer is expired */
		timer->status = HAL_TIMER_STATUS_NOT_ATTACHED;
		return timer->status;
	}
	else

		/* The status must be HAL_TIMER_STATUS_NOT_ATTACHED */
		return timer->status;
}

void hal_timer_update_freq(uint32_t clk_freq_hz)
{
	TIM_TypeDef *base;
	uint32_t auto_reload;
	uint32_t prescaler;

	for(hal_timer_timer_inst_e inst = 0; inst < HAL_TIMER_TIMER_INST_MAX; inst++)
	{
		/* Update prescaler and auto-reload values only if the timer has been
		 * configured */
		if(vhal_timer_tick_freq_hz[inst] > 0)
		{
			base = vhal_timer_base[inst];

			hal_timer_calc_prescaler_counts(clk_freq_hz,
											vhal_timer_tick_freq_hz[inst],
											inst,
											&prescaler,
											&auto_reload);

			/* Configure prescaler register */
			base->PSC = prescaler;

			/* Configure auto-reload register */
			base->ARR = auto_reload;
		}
	}

}

static void hal_timer_calc_prescaler_counts(uint32_t clk_freq_hz,
											uint32_t tick_freq_hz,
											hal_timer_timer_inst_e timer_inst,
											uint32_t *prescaler,
											uint32_t *auto_reload)
{
	uint32_t max_counts;

	/* Calculate prescaler and auto-reload values */

	max_counts = chal_timer_max_count[timer_inst];

	*prescaler = 0;

	*auto_reload = clk_freq_hz / tick_freq_hz - 1;

	/* If the auto-preload value is too big, then the prescaler needs to be
	 * used */

	if(*auto_reload > max_counts)
	{
		*prescaler = (*auto_reload + 1) / max_counts;

		*auto_reload = (*auto_reload + 1) / (*prescaler + 1) - 1;
	}
}

static void hal_timer_enable_clk(hal_timer_timer_inst_e timer_inst)
{
	if(timer_inst == HAL_TIMER_TIMER_INST_1)

		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_2)

		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_3)

		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_4)

		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_5)

		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_6)

		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_7)

		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_8)

		RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_15)

		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_16)

		RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;

	else if(timer_inst == HAL_TIMER_TIMER_INST_17)

		RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;

	else

		/* Do nothing */;

}

static inline void hal_timer_interrupt_handler(hal_timer_timer_inst_e timer_inst)
{
	TIM_TypeDef *base;

	base = vhal_timer_base[timer_inst];

	/* Clear interrupt flag */
	base->SR &= ~(TIM_SR_UIF);

	/* Increase tick counter */
	vhal_timer_ticks[timer_inst]++;
}

void TIM1_UP_TIM16_IRQHandler(void)
{
	/* Which timer triggered the interrupt? */

	if((TIM1->DIER & TIM_DIER_UIE) == TIM_DIER_UIE)

		hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_1);

	else if((TIM16->DIER & TIM_DIER_UIE) == TIM_DIER_UIE)

		hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_16);
}

void TIM2_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_2);
}

void TIM3_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_3);
}

void TIM4_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_4);
}

void TIM5_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_5);
}

void TIM6_DACUNDER_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_6);
}

void TIM7_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_7);
}

void TIM8_UP_IRQHandler(void)
{
	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_8);
}

void TIM1_BRK_TIM15_IRQHandler(void)
{
	/* Timer 1 can't trigger this interrupt */

	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_15);
}

/* Timer 16 interrupt is served in TIM1_UP_TIM16_IRQHandler, together with timer
 * 1 up interrupt */

void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
	/* Timer 1 can't trigger this interrupt */

	hal_timer_interrupt_handler(HAL_TIMER_TIMER_INST_17);
}


TIM_TypeDef *vhal_timer_base[HAL_TIMER_TIMER_INST_MAX] =
{
	TIM1,
	TIM2,
	TIM3,
	TIM4,
	TIM5,
	TIM6,
	TIM7,
	TIM8,
	TIM15,
	TIM16,
	TIM17
};

const uint32_t chal_timer_max_count[HAL_TIMER_TIMER_INST_MAX] =
{
	0xFFFF,		// HAL_TIMER_TIMER_INST_1
	0xFFFFFFFF,	// HAL_TIMER_TIMER_INST_2
	0xFFFFFFFF,	// HAL_TIMER_TIMER_INST_3
	0xFFFFFFFF,	// HAL_TIMER_TIMER_INST_4
	0xFFFFFFFF,	// HAL_TIMER_TIMER_INST_5
	0xFFFF,		// HAL_TIMER_TIMER_INST_6
	0xFFFF,		// HAL_TIMER_TIMER_INST_7
	0xFFFF,		// HAL_TIMER_TIMER_INST_8
	0xFFFF,		// HAL_TIMER_TIMER_INST_15
	0xFFFF,		// HAL_TIMER_TIMER_INST_16
	0xFFFF		// HAL_TIMER_TIMER_INST_17
};

const IRQn_Type chal_timer_interrupt_source[HAL_TIMER_TIMER_INST_MAX] =
{
		TIM1_UP_TIM16_IRQn,			// HAL_TIMER_TIMER_INST_1
		TIM2_IRQn,					// HAL_TIMER_TIMER_INST_2
		TIM3_IRQn,					// HAL_TIMER_TIMER_INST_3
		TIM4_IRQn,					// HAL_TIMER_TIMER_INST_4
		TIM5_IRQn,					// HAL_TIMER_TIMER_INST_1
		TIM6_DAC_IRQn,				// HAL_TIMER_TIMER_INST_1
		TIM7_IRQn,					// HAL_TIMER_TIMER_INST_1
		TIM8_UP_IRQn,				// HAL_TIMER_TIMER_INST_1
		TIM1_BRK_TIM15_IRQn,		// HAL_TIMER_TIMER_INST_1
		TIM1_UP_TIM16_IRQn,			// HAL_TIMER_TIMER_INST_1
		TIM1_TRG_COM_TIM17_IRQn,	// HAL_TIMER_TIMER_INST_1

};
