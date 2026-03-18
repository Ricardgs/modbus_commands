/*
 * drv_clk_mng.c
 *
 *  Created on: Mar 6, 2026
 *      Author: ricard
 */
#include <drv_clk_mng/drv_clk_mng.h>
#include "hal_clk/hal_clk.h"
#include "hal_uart/hal_uart.h"
#include "hal_timer/hal_timer.h"
#include "error.h"
#include <stdbool.h>

typedef enum
{
	DRV_CLK_MNG_STATE_IDLE,
	DRV_CLK_MNG_STATE_DISABLE_UARTS,
	DRV_CLK_MNG_STATE_UPDATE_FREQUENCY,
	DRV_CLK_MNG_STATE_MAX
} drv_clk_mng_state_e;

drv_clk_mng_state_e vdrv_clk_mng_state;
uint32_t vdrv_clk_mng_requested_frequency_hz,
		 vdrv_clk_mng_prev_requested_frequency_hz;

void drv_clk_mng_init(void)
{
	vdrv_clk_mng_state = DRV_CLK_MNG_STATE_IDLE;

	vdrv_clk_mng_requested_frequency_hz = HAL_CLK_TARGET_FREQ_HZ;

	vdrv_clk_mng_prev_requested_frequency_hz = HAL_CLK_TARGET_FREQ_HZ;
}

void drv_clk_mng_start(void)
{
	/* Nothing to be done here */
}

void drv_clk_mng_fxn(void)
{
	uint32_t actual_freq_hz;
	hal_uart_uart_num_e uart;

	switch(vdrv_clk_mng_state)
	{
	case DRV_CLK_MNG_STATE_IDLE:

		if((vdrv_clk_mng_requested_frequency_hz != vdrv_clk_mng_prev_requested_frequency_hz)
				&& (vdrv_clk_mng_requested_frequency_hz >= HAL_CLK_MIN_FREQ_HZ)
				&& (vdrv_clk_mng_requested_frequency_hz <= HAL_CLK_MAX_FREQ_HZ))

			/* A new valid frequency has been requested */
			vdrv_clk_mng_state = DRV_CLK_MNG_STATE_DISABLE_UARTS;

		break;

	case DRV_CLK_MNG_STATE_DISABLE_UARTS:

		for(uart = 0; uart < HAL_UART_UART_MAX; uart++)

			hal_uart_disable(uart);

		for(uart = 0; uart < HAL_UART_UART_MAX; uart++)

			if(hal_uart_is_disabled(uart) == false)

				break;

		if(uart == HAL_UART_UART_MAX)

			/* All UARTs are disabled. Proceed to change the clock */
			vdrv_clk_mng_state = DRV_CLK_MNG_STATE_UPDATE_FREQUENCY;


		break;

	case DRV_CLK_MNG_STATE_UPDATE_FREQUENCY:

		/* First disable the timers */
		hal_timer_stop_all();

		/* With the UARTs disabled and timers stopped, proceed to modify the
		 * clock frequency */
		(void)hal_clk_set_freq_hz(vdrv_clk_mng_requested_frequency_hz);

		vdrv_clk_mng_prev_requested_frequency_hz
			= vdrv_clk_mng_requested_frequency_hz;

		actual_freq_hz = hal_clk_get_freq_hz();

		hal_timer_update_freq(actual_freq_hz);
		hal_uart_update_clk_freq(actual_freq_hz);

		/* Enable UARTs */

		for(uart = 0; uart < HAL_UART_UART_MAX; uart++)

			hal_uart_enable(uart);

		/* Enable timers */
		hal_timer_resume_all();

		vdrv_clk_mng_state = DRV_CLK_MNG_STATE_IDLE;

		break;

	default:

		break;

	}
}

void drv_clk_mng_set_request(uint32_t freq_hz)
{
	vdrv_clk_mng_requested_frequency_hz = freq_hz;
}
