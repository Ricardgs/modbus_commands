/*
 * drv_modbus_registers.h
 *
 *  Created on: Feb 17, 2026
 *      Author: ricard
 */

#ifndef DRV_DRV_MODBUS_DRV_MODBUS_REGISTERS_H_
#define DRV_DRV_MODBUS_DRV_MODBUS_REGISTERS_H_

#include <stdint.h>
#include "drv_modbus_common.h"
#include "error.h"

/* Types */

enum
{
	DRV_MODBUS_0_INPUT_REG_PUSH_BUTTON,
	DRV_MODBUS_0_INPUT_REG_MAX
};

enum
{
	DRV_MODBUS_0_HOLDING_REG_CLK_FREQ_HIGH,
	DRV_MODBUS_0_HOLDING_REG_CLK_FREQ_LOW,
	DRV_MODBUS_0_HOLDING_REG_BAUDRATE_HIGH,
	DRV_MODBUS_0_HOLDING_REG_BAUDRATE_LOW,
	DRV_MODBUS_0_HOLDING_REG_LED,
	DRV_MODBUS_0_HOLDING_REG_MAX
};

typedef enum
{
	DRV_MODBUS_REGISTER_TYPE_INPUT,
	DRV_MODBUS_REGISTER_TYPE_HOLDING
} drv_modbus_register_type_s;

/* Constants */

extern const uint16_t vdrv_modbus_0_input_regs_addr[DRV_MODBUS_0_INPUT_REG_MAX];
extern const uint16_t vdrv_modbus_0_holding_regs_addr[DRV_MODBUS_0_HOLDING_REG_MAX];
extern uint16_t vdrv_modbus_0_input_regs_val[DRV_MODBUS_0_INPUT_REG_MAX];
extern uint16_t vdrv_modbus_0_holding_regs_val[DRV_MODBUS_0_HOLDING_REG_MAX];

/* APIs */
error_e drv_modbus_read_register(drv_modbus_inst inst,
								 drv_modbus_register_type_s type,
								 uint16_t reg,
								 uint16_t *val);
error_e drv_modbus_write_register(drv_modbus_inst inst,
		 	 	 	 	 	 	  drv_modbus_register_type_s type,
								  uint16_t reg,
								  uint16_t val);

#endif /* DRV_DRV_MODBUS_DRV_MODBUS_REGISTERS_H_ */
