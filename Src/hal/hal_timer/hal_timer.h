/*
 * hal_timer.h
 *
 *  Created on: Jan 19, 2026
 *      Author: ricard
 */

#ifndef HAL_HAL_TIMER_HAL_TIMER_H_
#define HAL_HAL_TIMER_HAL_TIMER_H_

#include <stdint.h>
#include <error.h>

typedef enum
{
	HAL_TIMER_TIMER_INST_1,
	HAL_TIMER_TIMER_INST_2,
	HAL_TIMER_TIMER_INST_3,
	HAL_TIMER_TIMER_INST_4,
	HAL_TIMER_TIMER_INST_5,
	HAL_TIMER_TIMER_INST_6,
	HAL_TIMER_TIMER_INST_7,
	HAL_TIMER_TIMER_INST_8,
	HAL_TIMER_TIMER_INST_15,
	HAL_TIMER_TIMER_INST_16,
	HAL_TIMER_TIMER_INST_17,
	HAL_TIMER_TIMER_INST_MAX
} hal_timer_timer_inst_e;

typedef struct
{
	hal_timer_timer_inst_e timer_inst;
	uint32_t clk_freq_hz;
	uint32_t tick_freq_hz;
} hal_timer_config_s;

typedef enum
{
	HAL_TIMER_STATUS_NOT_ATTACHED,
	HAL_TIMER_STATUS_TIMEOUT_NOT_REACHED,
	HAL_TIMER_STATUS_TIMEOUT_REACHED
} hal_timer_status_e;

typedef struct
{
	hal_timer_timer_inst_e inst;
	hal_timer_status_e status;
	uint32_t time_ref;
	uint32_t diff;
} hal_timer_timer_s;

void hal_timer_init(void);
void hal_timer_start(hal_timer_config_s config);
error_e hal_timer_attach(hal_timer_timer_inst_e timer_inst,
					  	 hal_timer_timer_s *timer,
						 uint32_t timeout_ms);
void hal_timer_detach(hal_timer_timer_s *timer);
hal_timer_status_e hal_timer_status_get(hal_timer_timer_s *timer);
void hal_timer_update_freq(uint32_t clk_freq_hz);

#endif /* HAL_HAL_TIMER_HAL_TIMER_H_ */
