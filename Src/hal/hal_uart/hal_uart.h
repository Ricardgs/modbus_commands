/*
 * hal_uart.h
 *
 *  Created on: Jan 18, 2026
 *      Author: ricard
 */

#ifndef HAL_HAL_UART_HAL_UART_H_
#define HAL_HAL_UART_HAL_UART_H_

#include <stdint.h>
#include <error.h>

typedef enum
{
	HAL_UART_USART_2,
	HAL_UART_UART_MAX
} hal_uart_uart_num_e;

typedef enum
{
	HAL_UART_CIRC_BUFF_DIR_RX,
	HAL_UART_CIRC_BUFF_DIR_TX,
	HAL_UART_CIRC_BUFF_DIR_MAX
} hal_uart_circ_buff_dir_e;

typedef enum
{
	HAL_UART_PARITY_NONE,
	HAL_UART_PARITY_ODD,
	HAL_UART_PARITY_EVEN,
	HAL_UART_PARITY_MAX
} hal_uart_parity_s;

typedef enum
{
	HAL_UART_STOP_BITS_0_5,
	HAL_UART_STOP_BITS_1,
	HAL_UART_STOP_BITS_1_5,
	HAL_UART_STOP_BITS_2,
	HAL_UART_STOP_BITS_MAX
} hal_uart_stop_bits_s;

typedef enum
{
	HAL_UART_N_BITS_6,
	HAL_UART_N_BITS_7,
	HAL_UART_N_BITS_8,
	HAL_UART_N_BITS_9,
	HAL_UART_N_BITS_MAX
} hal_uart_n_bits_s;

typedef struct
{
	hal_uart_uart_num_e uart_num;
	uint32_t baudrate;
	uint32_t clk_freq_hz;
	hal_uart_parity_s parity			: 2;
	hal_uart_stop_bits_s n_stop_bits	: 3;
	hal_uart_n_bits_s n_bits			: 3;
} hal_uart_config_s;

void hal_uart_init(void);
void hal_uart_start(hal_uart_config_s config);
error_e hal_uart_send(hal_uart_uart_num_e uart_num,
					  uint8_t *buf,
					  uint8_t len);
error_e hal_uart_retrieve(hal_uart_uart_num_e uart_num,
						  uint8_t *buf,
						  uint8_t len);
void hal_uart_flush_buffer(hal_uart_uart_num_e uart_num);

#endif /* HAL_HAL_UART_HAL_UART_H_ */
