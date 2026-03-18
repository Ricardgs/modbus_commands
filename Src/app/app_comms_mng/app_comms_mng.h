/*
 * app_comms_mng.h
 *
 *  Created on: Feb 19, 2026
 *      Author: ricard
 */

#ifndef APP_APP_COMMS_MNG_H_
#define APP_APP_COMMS_MNG_H_

#include "drv_modbus/drv_modbus_common.h"
#include <stdint.h>

void app_comms_mng_init(void);
void app_comms_mng_start(void);
void app_comms_mng_fxn(void);
uint32_t app_comms_mng_baurdate_request_get(drv_modbus_inst modbus_inst);

#endif /* APP_APP_COMMS_MNG_H_ */
