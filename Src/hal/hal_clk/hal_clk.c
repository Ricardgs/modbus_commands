/*
 * hal_clk.c
 *
 *  Created on: Jan 15, 2026
 *      Author: ricard
 */


#include "hal_clk.h"
#include <stm32l476xx.h>
#include <core_cm4.h>

#define HAL_CLK_MIN_FREQ_HZ								8000000
#define HAL_CLK_MAX_FREQ_HZ								80000000

/* Range of possible VCO input frequencies */
#define HAL_CLK_VCO_IN_MAX_FREQ_HZ						16000000
#define HAL_CLK_VCO_IN_MIN_FREQ_HZ						4000000

/* Range of possible VCO output frequencies */
#define HAL_CLK_VCO_OUT_MAX_FREQ_HZ						344000000
#define HAL_CLK_VCO_OUT_MIN_FREQ_HZ						64000000

#define HAL_CLK_MAX_PLLN								86
#define HAL_CLK_MIN_PLLN								8

#define HAL_CLK_MAX_PLLM								8
#define HAL_CLK_MIN_PLLM								1

#define HAL_CLK_PLLR_TOTAL_VALUES						4

/* This frequencies correspond to Vcore Range 1 */
#define HAL_CLK_0_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ	16000000
#define HAL_CLK_1_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ	32000000
#define HAL_CLK_2_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ	48000000
#define HAL_CLK_3_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ	64000000
#define HAL_CLK_4_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ	80000000

/* Functions */
static void hal_clk_adjust_flash_wait_states(uint32_t freq_hz);

/* Variables */

static uint32_t vhal_clk_freq_hz;
static const uint8_t chal_clk_pllr_values[HAL_CLK_PLLR_TOTAL_VALUES] = { 2, 4, 6, 8 };

void hal_clk_init(void)
{
	vhal_clk_freq_hz = HAL_CLK_DEFAULT_FREQ_HZ;
}

void hal_clk_start(void)
{
	/* Nothing to be done */
}

error_e hal_clk_set_freq_hz(uint32_t freq_hz)
{
	float freq_factor, freq_factor_mult_by_pllr;
	uint32_t vco_input_freq, vco_output_freq;
	uint8_t pllr_index, plln, pllm, pllr;
	uint32_t new_freq_hz;

	if(freq_hz < HAL_CLK_MIN_FREQ_HZ || freq_hz > HAL_CLK_MAX_FREQ_HZ)

		return ERROR_INCORRECT_FREQ;

	else if(freq_hz == vhal_clk_freq_hz)

		return ERROR_NONE;

	else
	{
		freq_factor = (float)freq_hz/(float)HAL_CLK_MSI_FREQ_HZ;

		for(pllr_index = 0;
			pllr_index < HAL_CLK_PLLR_TOTAL_VALUES;
			pllr_index++)
		{
			pllr = chal_clk_pllr_values[pllr_index];

			freq_factor_mult_by_pllr = freq_factor * pllr;

			/* PLLM can have limited values, and VCO input frequency must be
			 * inside range */
			for(pllm = HAL_CLK_MIN_PLLM;
				pllm <= HAL_CLK_MAX_PLLM
					&& (vco_input_freq = HAL_CLK_MSI_FREQ_HZ / pllm) >= HAL_CLK_VCO_IN_MIN_FREQ_HZ
					&& vco_input_freq <= HAL_CLK_VCO_IN_MAX_FREQ_HZ;
				pllm++)
			{
				plln = freq_factor_mult_by_pllr * pllm;

				vco_output_freq = vco_input_freq * plln;

				/* PLLN can have limited values, and VCO output frequency must be
				 * inside range */
				if(plln >= HAL_CLK_MIN_PLLN
					&& plln <= HAL_CLK_MAX_PLLN
					&& vco_output_freq >= HAL_CLK_VCO_OUT_MIN_FREQ_HZ
					&& vco_output_freq <= HAL_CLK_VCO_OUT_MAX_FREQ_HZ)
				{
					/* A feasible set of values have been found! Before updating
					 * PLL configuration, first select MSI as the clock source
					 * */

					if(HAL_CLK_MSI_FREQ_HZ < vhal_clk_freq_hz)
					{
						/* Switching to MSI clock means the frequency is being
						 * decreased. Switch the clock source first, and then
						 * modify the number of wait states */

						/* Disable interrupts in order to prevent the
						 * vhal_clk_freq_hz variable from containing an
						 * incorrect value */
						__disable_irq();

						/* Select MSI as the clock source for SYSCLK */
						RCC->CFGR &= ~(RCC_CFGR_SW_Msk);

						/* Update variable */
						vhal_clk_freq_hz = HAL_CLK_MSI_FREQ_HZ;

						/* Re-nable interrupts */
						__enable_irq();

						/* Adjust the number of wait states when reading flash
						 * memory */
						hal_clk_adjust_flash_wait_states(vhal_clk_freq_hz);
					}
					else
					{
						/* Switching to MSI clock means the frequency is being
						 * increased. Modify the number of wait states first,
						 * and then switch the clock source */

						/* Disable interrupts */
						__disable_irq();

						/* Adjust the number of wait states when reading flash
						 * memory */
						hal_clk_adjust_flash_wait_states(vhal_clk_freq_hz);

						/* Select MSI as the clock source for SYSCLK */
						RCC->CFGR &= ~(RCC_CFGR_SW_Msk);

						/* Update variable */
						vhal_clk_freq_hz = HAL_CLK_MSI_FREQ_HZ;

						/* Re-nable interrupts */
						__enable_irq();
					}

					/* In order to modify the PLL configuration, it is required
					 * to first shut it down */

					/* Disable PLL */
					RCC->CR &= ~(RCC_CR_PLLON_Msk);

					/* Wait until PLLRDY flag is cleared */
					while(RCC->CR & RCC_CR_PLLRDY_Msk);

					/* Select MSI clock as the source for PLL */
					RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC_Msk;
					RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_MSI;

					/* PLL is now disabled. Proceed to change its configuration */

					RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLR_Msk);
					RCC->PLLCFGR |= (((uint32_t)pllr_index) << RCC_PLLCFGR_PLLR_Pos)
									& RCC_PLLCFGR_PLLR_Msk;

					RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_Msk);
					RCC->PLLCFGR |= ((uint32_t)(pllm - 1) << RCC_PLLCFGR_PLLM_Pos)
									& RCC_PLLCFGR_PLLM_Msk;

					RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN_Msk);
					RCC->PLLCFGR |= (((uint32_t)plln) << RCC_PLLCFGR_PLLN_Pos)
									& RCC_PLLCFGR_PLLN_Msk;

					/* Enable R output of main PLL */
					RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;

					/* The PLL is now configured. Re-enable it */
					RCC->CR |= RCC_CR_PLLON_Msk;

					/* Wait until PLLRDY flag is set */
					while((RCC->CR & RCC_CR_PLLRDY_Msk) == 0);

					/* Enable the R output */
					RCC->CR |= RCC_PLLCFGR_PLLREN_Msk;

					/* Before selecting PLL as the clock source, the number of
					 * wait states (WS) in flash memory needs to be adjusted */

					new_freq_hz =
							HAL_CLK_MSI_FREQ_HZ * plln
							/ (pllm * pllr);

					if(new_freq_hz < vhal_clk_freq_hz)
					{
						/* Switching to PLL clock means the frequency is being
						 * decreased. Switch the clock source first, and then
						 * modify the number of wait states */

						/* Disable interrupts */
						__disable_irq();

						/* Select MSI as the clock source for SYSCLK */
						RCC->CFGR &= ~(RCC_CFGR_SW_Msk);

						/* Update variable containing the frequency */
						vhal_clk_freq_hz = new_freq_hz;

						/* Select PLL as the clock source for SYSCLK */
						RCC->CFGR &= ~RCC_CFGR_SW;
						RCC->CFGR |= RCC_CFGR_SW_PLL;

						/* Adjust the number of wait states when reading flash
						 * memory */
						hal_clk_adjust_flash_wait_states(vhal_clk_freq_hz);

						/* Re-nable interrupts */
						__enable_irq();
					}
					else
					{
						/* Switching to PLL clock means the frequency is being
						 * increased. Modify the number of wait states first,
						 * and then switch the clock source */

						/* Disable interrupts */
						__disable_irq();

						/* Adjust the number of wait states when reading flash
						 * memory */
						hal_clk_adjust_flash_wait_states(new_freq_hz);

						/* Select MSI as the clock source for SYSCLK */
						RCC->CFGR &= ~(RCC_CFGR_SW_Msk);

						/* Update variable containing the frequency */
						vhal_clk_freq_hz = new_freq_hz;

						/* Select PLL as the clock source for SYSCLK */
						RCC->CFGR &= ~RCC_CFGR_SW;
						RCC->CFGR |= RCC_CFGR_SW_PLL;

						/* Re-nable interrupts */
						__enable_irq();
					}

					return ERROR_NONE;
				}
			}
		}

		/* Could not match frequency */
		return ERROR_INCORRECT_FREQ;
	}
}

uint16_t hal_clk_get_freq_hz(void)
{
	return vhal_clk_freq_hz;
}

/********************************** WARNING ***********************************/
/* Interrupts must be disabled when calling this function!!!!!!!! */
static void hal_clk_adjust_flash_wait_states(uint32_t freq_hz)
{
	uint32_t acr = FLASH->ACR & ~FLASH_ACR_LATENCY;

	if(freq_hz <= HAL_CLK_0_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ)
	{
		/* 0 wait states */
		acr |= FLASH_ACR_LATENCY_0WS;
	}
	else if(freq_hz <= HAL_CLK_1_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ)
	{
		/* 1 wait state */
		acr |= FLASH_ACR_LATENCY_1WS;
	}
	else if(freq_hz <= HAL_CLK_2_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ)
	{
		/* 2 wait state */
		acr |= FLASH_ACR_LATENCY_2WS;
	}
	else if(freq_hz <= HAL_CLK_3_WAIT_STATES_VCORE_RANGE_1_MAX_FREQ_HZ)
	{
		/* 3 wait state */
		acr |= FLASH_ACR_LATENCY_3WS;
	}
	else
	{
		/* 4 wait state */
		acr |= FLASH_ACR_LATENCY_4WS;
	}

	FLASH->ACR = acr;

	/* Ensure the new value is taken into account */
	while((FLASH->ACR & FLASH_ACR_LATENCY) != (acr & FLASH_ACR_LATENCY));
}
