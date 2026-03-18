/*
 * app_data_handler.c
 *
 *  Created on: Mar 18, 2026
 *      Author: ricard
 */


#include "app_data_handler.h"
#include "drv_modbus/drv_modbus_common.h"
#include "app_comms_mng/app_comms_mng.h"
#include "app_baudrate_mng/app_baudrate_mng.h"

void app_data_handler_init(void)
{
	/* Nothing to be done */
}

void app_data_handler_start(void)
{
	/* Nothing to be done */
}

void app_data_handler_fxn(void)
{
	uint32_t data_32;

	data_32 = app_comms_mng_baurdate_request_get(DRV_MODBUS_INST_0);

	app_baudrate_new_baudrate_set(DRV_MODBUS_INST_0, data_32);
}
