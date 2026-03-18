/*
 * drv_clk_mng.h
 *
 *  Created on: Mar 6, 2026
 *      Author: ricard
 */

#ifndef DRV_DRV_CLK_MNG_DRV_CLK_MNG_H_
#define DRV_DRV_CLK_MNG_DRV_CLK_MNG_H_

#include <stdint.h>

void drv_clk_mng_init(void);
void drv_clk_mng_start(void);
void drv_clk_mng_fxn(void);
void drv_clk_mng_set_request(uint32_t freq_hz);

#endif /* DRV_DRV_CLK_MNG_DRV_CLK_MNG_H_ */
