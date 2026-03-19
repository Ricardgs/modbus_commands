/*
 * config.c
 *
 *  Created on: Feb 20, 2026
 *      Author: ricard
 */
#include "stdlib.h"
#include "config.h"
#include "hal_os/hal_os.h"
#include "hal_clk/hal_clk.h"
#include "hal_pin_mat/hal_pin_mat.h"
#include "hal_uart/hal_uart.h"
#include "hal_gpio/hal_gpio.h"
#include "hal_timer/hal_timer.h"
#include "drv_led/drv_led.h"
#include "drv_push_button/drv_push_button.h"
#include "drv_modbus/drv_modbus.h"
#include "drv_clk_mng/drv_clk_mng.h"
#include "drv_modbus/drv_modbus_common.h"
#include "app_comms_mng/app_comms_mng.h"
#include "app_baudrate_mng/app_baudrate_mng.h"
#include "app_data_handler/app_data_handler.h"

typedef enum
{
	/* HAL - most are not actually tasks, because they have no fxn */
	CONFIG_TASK_PIN_MAT,
	CONFIG_TASK_UART,
	CONFIG_TASK_GPIO,
	CONFIG_TASK_TIMER,
	/* DRV */
	CONFIG_TASK_LED,
	CONFIG_TASK_PUSH_BUTTON,
	CONFIG_TASK_MODBUS,
	CONFIG_TASK_CLK_MNG,
	/* APP */
	CONFIG_TASK_COMMS_MNG,
	CONFIG_TASK_BAUDRATE_MNG,
	CONFIG_TASK_DATA_HANDLER,

	CONFIG_TASK_MAX
} config_tasks_e;

typedef struct
{
	void (*init)(void);
	void (*start)(void);
	void (*fxn)(void);
} config_task_s;

void config_uart_start(void);
void config_timer_start(void);
void config_led_start(void);
void config_push_button_start(void);
void config_modbus_start(void);
void config_baudrate_mng_start(void);

extern const config_task_s cconfig_task[CONFIG_TASK_MAX];

const hal_uart_config_s cconfig_uart[HAL_UART_UART_MAX] =
{
		{
				.uart_num = HAL_UART_USART_2,
				.baudrate = DRV_MODBUS_DEFAULT_BAUDRATE,
				.clk_freq_hz = HAL_CLK_TARGET_FREQ_HZ,
				.parity = DRV_MODBUS_DEFAULT_PARITY,
				.n_stop_bits = DRV_MODBUS_DEFAULT_STOP_BITS,
				.n_bits = DRV_MODBUS_DEFAULT_N_BITS
		}
};

const hal_timer_config_s cconfig_timer[] =
{
		{
				.timer_inst = HAL_TIMER_TIMER_INST_6,
				.clk_freq_hz = HAL_CLK_TARGET_FREQ_HZ,
				.tick_freq_hz = 1000
		}
};

const drv_led_congif_s cconfig_led[DRV_LED_INST_MAX] =
{
		{
				.led_inst = DRV_LED_INST_0,
				.gpio_inst = HAL_GPIO_INST_A,
				.pin = 5,
				.timer_inst = HAL_TIMER_TIMER_INST_6
		}
};

const drv_hal_push_button_config_s cconfig_push_button[DRV_PUSH_BUTTON_MAX] =
{
		{
				.push_button_inst = DRV_PUSH_BUTTON_0,
				.gpio_inst = HAL_GPIO_INST_C,
				.pin = 13
		}
};

const drv_modbus_config_s cconfig_modbus[DRV_MODBUS_INST_MAX] =
{
		{
				.inst = DRV_MODBUS_INST_0,
				.uart_inst = HAL_UART_USART_2,
				.timer_inst = HAL_TIMER_TIMER_INST_6,
				.mb_addr = 0x10
		}
};

const app_baudrate_mng_config_t cconfig_baudrate_mng[DRV_MODBUS_INST_MAX] =
{
		{
				.modbus_inst = DRV_MODBUS_INST_0,
				.uart_num = HAL_UART_USART_2,
				.baudrate = DRV_MODBUS_DEFAULT_BAUDRATE
		}
};

const config_task_s cconfig_task[CONFIG_TASK_MAX] =
{
		{	.init = hal_pin_mat_init,		.start = hal_pin_mat_start,			.fxn = NULL					},	// CONFIG_TASK_PIN_MAT
		{	.init = hal_uart_init,			.start = config_uart_start,			.fxn = NULL					},	// CONFIG_TASK_UART
		{	.init = hal_gpio_init,			.start = hal_gpio_start,			.fxn = NULL					},	// CONFIG_TASK_GPIO
		{	.init = hal_timer_init,			.start = config_timer_start,		.fxn = NULL					},	// CONFIG_TASK_TIMER
		{	.init = drv_led_init,			.start = config_led_start,			.fxn = drv_led_fxn			},	// CONFIG_TASK_LED
		{	.init = drv_push_button_init,	.start = config_push_button_start,	.fxn = drv_push_button_fxn	},	// CONFIG_TASK_PUSH_BUTTON
		{	.init = drv_modbus_init,		.start = config_modbus_start,		.fxn = drv_modbus_fxn		},	// CONFIG_TASK_MODBUS
		{	.init = drv_clk_mng_init,		.start = drv_clk_mng_start,			.fxn = drv_clk_mng_fxn		},	// CONFIG_TASK_MODBUS
		{	.init = app_comms_mng_init,		.start = app_comms_mng_start,		.fxn = app_comms_mng_fxn	},	// CONFIG_TASK_COMMS_MNG
		{	.init = app_baudrate_mng_init,	.start = config_baudrate_mng_start,	.fxn = app_baudrate_mng_fxn	},	// CONFIG_TASK_BAUDRATE_MNG
		{	.init = app_data_handler_init,	.start = app_data_handler_start,	.fxn = app_data_handler_fxn	},	// CONFIG_TASK_DATA_HANDLER
};

void config_init_tasks(void)
{
	for(config_tasks_e i = 0; i < CONFIG_TASK_MAX; i++)
	{
		cconfig_task[i].init();
		cconfig_task[i].start();

		if(cconfig_task[i].fxn != NULL)

			hal_os_task_attach(cconfig_task[i].fxn);
	}
}

void config_uart_start(void)
{
	for(int i = 0; i < HAL_UART_UART_MAX; i++)

		hal_uart_start(&cconfig_uart[i]);
}

void config_timer_start(void)
{
	for(int i = 0; i < sizeof(cconfig_timer) / sizeof(hal_timer_config_s); i++)

		hal_timer_start(&cconfig_timer[i]);
}

void config_led_start(void)
{
	for(int i = 0; i < DRV_LED_INST_MAX; i++)

		drv_led_start(&cconfig_led[i]);
}

void config_push_button_start(void)
{
	for(int i = 0; i < DRV_PUSH_BUTTON_MAX; i++)

		drv_push_button_start(&cconfig_push_button[i]);
}

void config_modbus_start(void)
{
	for(int i = 0; i < DRV_MODBUS_INST_MAX; i++)

		drv_modbus_start(&cconfig_modbus[i]);
}

void config_baudrate_mng_start(void)
{
	for(int i = 0; i < DRV_MODBUS_INST_MAX; i++)

		app_baudrate_mng_start(&cconfig_baudrate_mng[i]);
}
