/*
 * drv_modbus.h
 *
 *  Created on: Jan 24, 2026
 *      Author: ricard
 */

#ifndef DRV_DRV_MODBUS_DRV_MODBUS_H_
#define DRV_DRV_MODBUS_DRV_MODBUS_H_

#include <stdint.h>
#include "drv_modbus_common.h"
#include "../../hal/hal_uart/hal_uart.h"
#include "../../hal/hal_timer/hal_timer.h"

typedef struct
{
	drv_modbus_inst inst;
	hal_uart_uart_num_e uart_inst;
	hal_timer_timer_inst_e timer_inst;
	uint8_t mb_addr;
} drv_modbus_config_s;


void drv_modbus_init(void);
void drv_modbus_start(const drv_modbus_config_s config);
void drv_modbus_fxn(void);

#endif /* DRV_DRV_MODBUS_DRV_MODBUS_H_ */
