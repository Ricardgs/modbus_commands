/*
 * hal_clk.h
 *
 *  Created on: Jan 15, 2026
 *      Author: ricard
 */

#ifndef HAL_HAL_CLK_H_
#define HAL_HAL_CLK_H_

#include <stdbool.h>
#include <stdint.h>
#include <error.h>

#define HAL_CLK_MSI_FREQ_HZ			4000000
#define HAL_CLK_DEFAULT_FREQ_HZ		HAL_CLK_MSI_FREQ_HZ

void hal_clk_init(void);
void hal_clk_start(void);
error_e hal_clk_set_freq_hz(uint32_t freq_hz);
uint16_t hal_clk_get_freq_hz(void);

#endif /* HAL_HAL_CLK_H_ */
