/*
 * app_baudrate_mng.h
 *
 *  Created on: Mar 18, 2026
 *      Author: ricard
 */

#ifndef APP_APP_BAUDRATE_MNG_APP_BAUDRATE_MNG_H_
#define APP_APP_BAUDRATE_MNG_APP_BAUDRATE_MNG_H_

#include <stdint.h>
#include "hal_uart/hal_uart.h"
#include "drv_modbus/drv_modbus_common.h"

typedef struct
{
	drv_modbus_inst modbus_inst;
	hal_uart_uart_num_e uart_num;
	uint32_t baudrate;
} app_baudrate_mng_config_t;

void app_baudrate_mng_init(void);
void app_baudrate_mng_start(const app_baudrate_mng_config_t *config);
void app_baudrate_mng_fxn(void);
void app_baudrate_new_baudrate_set(drv_modbus_inst modbus_inst,
								   uint32_t baudrate);

#endif /* APP_APP_BAUDRATE_MNG_APP_BAUDRATE_MNG_H_ */
