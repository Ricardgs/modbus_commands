/*
 * drv_modbus_registers.c
 *
 *  Created on: Feb 17, 2026
 *      Author: ricard
 */


#include "drv_modbus_registers.h"

const uint16_t vdrv_modbus_0_input_regs_addr[DRV_MODBUS_0_INPUT_REG_MAX] =
{
		0x0000	// DRV_MODBUS_0_INPUT_REG_PUSH_BUTTON
};

const uint16_t vdrv_modbus_0_holding_regs_addr[DRV_MODBUS_0_HOLDING_REG_MAX] =
{
		0x0000,	// DRV_MODBUS_0_HOLDING_REG_CLK_FREQ_HIGH
		0x0001,	// DRV_MODBUS_0_HOLDING_REG_CLK_FREQ_LOW
		0x0002,	// DRV_MODBUS_0_HOLDING_REG_BAUDRATE_HIGH
		0x0003,	// DRV_MODBUS_0_HOLDING_REG_BAUDRATE_LOW
		0x0004	// DRV_MODBUS_0_HOLDING_REG_LED
};

uint16_t vdrv_modbus_0_input_regs_val[DRV_MODBUS_0_INPUT_REG_MAX];
uint16_t vdrv_modbus_0_holding_regs_val[DRV_MODBUS_0_HOLDING_REG_MAX];

error_e drv_modbus_read_register(drv_modbus_inst inst,
								 drv_modbus_register_type_s type,
								 uint16_t reg,
								 uint16_t *val)
{
	error_e ret;

	ret = ERROR_MODBUS_INEXISTENT_REGISTER;

	if(inst == DRV_MODBUS_INST_0)
	{
		if(type == DRV_MODBUS_REGISTER_TYPE_INPUT
			&& reg < DRV_MODBUS_0_INPUT_REG_MAX)
		{
			*val = vdrv_modbus_0_input_regs_val[reg];
			ret = ERROR_NONE;
		}
		else if(type == DRV_MODBUS_REGISTER_TYPE_HOLDING
				&& reg < DRV_MODBUS_0_HOLDING_REG_MAX)
		{
			*val = vdrv_modbus_0_holding_regs_val[reg];
			ret = ERROR_NONE;
		}
	}

	return ret;
}

error_e drv_modbus_write_register(drv_modbus_inst inst,
							      drv_modbus_register_type_s type,
								  uint16_t reg,
								  uint16_t val)
{
	error_e ret;

	ret = ERROR_MODBUS_INEXISTENT_REGISTER;

	if(inst == DRV_MODBUS_INST_0)
	{
		if(type == DRV_MODBUS_REGISTER_TYPE_INPUT
			&& reg < DRV_MODBUS_0_INPUT_REG_MAX)
		{
			vdrv_modbus_0_input_regs_val[reg] = val;
			ret = ERROR_NONE;
		}
		else if(type == DRV_MODBUS_REGISTER_TYPE_HOLDING
				&& reg < DRV_MODBUS_0_HOLDING_REG_MAX)
		{
			vdrv_modbus_0_holding_regs_val[reg] = val;
			ret = ERROR_NONE;
		}
	}

	return ret;
}
