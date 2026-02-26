/*
 * hal_uart.c
 *
 *  Created on: Jan 18, 2026
 *      Author: ricard
 */
#include <stm32l476xx.h>
#include "hal_uart.h"
#include "string.h"
#include <cmsis_gcc.h>
#include <core_cm4.h>
#include <stdio.h>

#define HAL_UART_BUFFER_DEPTH	100

typedef struct
{
	uint8_t buffer[HAL_UART_BUFFER_DEPTH];
	uint16_t in_ptr;
	uint16_t out_ptr;
} hal_uart_circ_buff_s;

static void hal_uart_enable_clk(USART_TypeDef *uart_inst);
static IRQn_Type hal_uart_get_interrupt_source(USART_TypeDef *uart_inst);
static void hal_uart_init_circular_buffer(hal_uart_uart_num_e uart_num);
static error_e hal_uart_set_baudrate(USART_TypeDef *uart_inst,
								  	 uint32_t clk_freq_hz,
								  	 uint32_t baudrate);
static error_e hal_uart_circ_buff_put_data(hal_uart_uart_num_e uart_num,
										   hal_uart_circ_buff_dir_e dir,
										   uint8_t *buf,
										   uint8_t len);
static error_e hal_uart_circ_buff_get_data(hal_uart_uart_num_e uart_num,
										   hal_uart_circ_buff_dir_e dir,
										   uint8_t *buf,
										   uint8_t len);
static void hal_uart_interrupt_handler(hal_uart_uart_num_e uart_num);

static hal_uart_circ_buff_s hal_uart_circ_buff[HAL_UART_UART_MAX][HAL_UART_CIRC_BUFF_DIR_MAX];

extern USART_TypeDef *hal_uart_inst[HAL_UART_UART_MAX];

void hal_uart_init(void)
{
	for(hal_uart_uart_num_e uart_num = 0; uart_num < HAL_UART_UART_MAX; uart_num++)
	{
		hal_uart_init_circular_buffer(uart_num);
	}
}

void hal_uart_start(hal_uart_config_s config)
{
	USART_TypeDef *uart_inst = hal_uart_inst[config.uart_num];

	/* Sanity check */

	if(config.uart_num >= HAL_UART_UART_MAX
		|| config.n_bits >= HAL_UART_N_BITS_MAX
		|| config.n_stop_bits >= HAL_UART_STOP_BITS_MAX
		|| config.parity >= HAL_UART_PARITY_MAX)

		return;

	/* If frame length is only 6 bits, then parity bit must be included. If
	 * frame length is 9 bits, then parity bit cannot be included. For other
	 * frame lengths, parity bit may be or may be not included */
	if((config.n_bits == HAL_UART_N_BITS_6 && config.parity == HAL_UART_PARITY_NONE)
		|| (config.n_bits == HAL_UART_N_BITS_9 && config.parity != HAL_UART_PARITY_NONE))

		return;

	hal_uart_enable_clk(uart_inst);

	/* Configure number of bits and parity */

	if(config.n_bits == HAL_UART_N_BITS_6)
	{
		/* With 6 bits of data, parity bit must be included */

		uart_inst->CR1 |= USART_CR1_M1;
		uart_inst->CR1 &= ~USART_CR1_M0;
		uart_inst->CR1 |= USART_CR1_PCE;

		if(config.parity == HAL_UART_PARITY_ODD)
		{
			/* Odd parity */
			uart_inst->CR1 |= USART_CR1_PS;
		}
		else
		{
			/* Even Parity */
			uart_inst->CR1 &= ~USART_CR1_PS;
		}
	}
	else if(config.n_bits == HAL_UART_N_BITS_7)
	{
		/* With 7 bits, parity may be or may be not included */

		if(config.parity == HAL_UART_PARITY_NONE)
		{
			/* No parity bit */
			uart_inst->CR1 |= USART_CR1_M1;
			uart_inst->CR1 &= ~USART_CR1_M0;
			uart_inst->CR1 &= ~USART_CR1_PCE;
		}
		else if(config.parity == HAL_UART_PARITY_ODD)
		{
			/* Odd parity */
			uart_inst->CR1 &= ~USART_CR1_M1;
			uart_inst->CR1 &= ~USART_CR1_M0;
			uart_inst->CR1 |= USART_CR1_PCE;
			uart_inst->CR1 |= USART_CR1_PS;
		}
		else
		{
			/* Even parity */
			uart_inst->CR1 &= ~USART_CR1_M1;
			uart_inst->CR1 &= ~USART_CR1_M0;
			uart_inst->CR1 |= USART_CR1_PCE;
			uart_inst->CR1 &= ~USART_CR1_PS;
		}
	}
	else if(config.n_bits == HAL_UART_N_BITS_8)
	{
		/* With 8 bits, parity may be or may be not included */

		if(config.parity == HAL_UART_PARITY_NONE)
		{
			/* No parity bit */
			uart_inst->CR1 &= ~USART_CR1_M1;
			uart_inst->CR1 &= ~USART_CR1_M0;
			uart_inst->CR1 &= ~USART_CR1_PCE;
		}
		else if(config.parity == HAL_UART_PARITY_ODD)
		{
			/* Odd parity */
			uart_inst->CR1 &= ~USART_CR1_M1;
			uart_inst->CR1 |= USART_CR1_M0;
			uart_inst->CR1 |= USART_CR1_PCE;
			uart_inst->CR1 |= USART_CR1_PS;
		}
		else
		{
			/* Even parity */
			uart_inst->CR1 &= ~USART_CR1_M1;
			uart_inst->CR1 |= USART_CR1_M0;
			uart_inst->CR1 |= USART_CR1_PCE;
			uart_inst->CR1 &= ~USART_CR1_PS;
		}
	}
	else
	{
		/* With 9 bits, parity can't be included */
		uart_inst->CR1 &= ~USART_CR1_M1;
		uart_inst->CR1 |= USART_CR1_M0;
		uart_inst->CR1 &= ~USART_CR1_PCE;

	}

	/* Stop bits */

	if(config.n_stop_bits == HAL_UART_STOP_BITS_0_5)
	{
		/* 0.5 stop bits */
		uart_inst->CR2 &= ~USART_CR2_STOP_1;
		uart_inst->CR2 |= USART_CR2_STOP_0;
	}
	else if(config.n_stop_bits == HAL_UART_STOP_BITS_1)
	{
		/* 1 stop bit */
		uart_inst->CR2 &= ~USART_CR2_STOP_1;
		uart_inst->CR2 &= ~USART_CR2_STOP_0;
	}
	else if(config.n_stop_bits == HAL_UART_STOP_BITS_1)
	{
		/* 1.5 stop bit */
		uart_inst->CR2 |= USART_CR2_STOP_1;
		uart_inst->CR2 |= USART_CR2_STOP_0;
	}
	else
	{
		/* 2 stop bits */
		uart_inst->CR2 |= USART_CR2_STOP_1;
		uart_inst->CR2 &= ~USART_CR2_STOP_0;
	}

	/* Set baudrate */
	(void)hal_uart_set_baudrate(uart_inst,
								config.clk_freq_hz,
								config.baudrate);

	/* Enable UART */
	uart_inst->CR1 |= USART_CR1_UE;

	/* Enable receiver */
	uart_inst->CR1 |= USART_CR1_RE;

	/* Enable RX interrupts */
	uart_inst->CR1 |= USART_CR1_RXNEIE;

	/* Enable transmitter */
	uart_inst->CR1 |= USART_CR1_TE;

	/* Enable interrupts in NVIC */
	NVIC_EnableIRQ(hal_uart_get_interrupt_source(uart_inst));
}

error_e hal_uart_send(hal_uart_uart_num_e uart_num,
					  uint8_t *buf,
					  uint8_t len)
{
	USART_TypeDef *uart_inst = hal_uart_inst[uart_num];
	error_e ret;

	/* Disable interrupts from the UART instance */
	NVIC_DisableIRQ(hal_uart_get_interrupt_source(uart_inst));

	/* Ensure the interrupt is disabled */
	__DSB();

	ret = hal_uart_circ_buff_put_data(uart_num, HAL_UART_CIRC_BUFF_DIR_TX, buf, len);

	/* If successful, start transmission and enable TX interrupt */
	if(ret == ERROR_NONE)
	{
		/* Start transmission */
		// uart_inst->TDR = hal_uart_circ_buff[uart_num][HAL_UART_CIRC_BUFF_DIR_TX].buffer[hal_uart_circ_buff[uart_num][HAL_UART_CIRC_BUFF_DIR_TX].out_ptr];

		/* Enable TXE interrupts */
		uart_inst->CR1 |= USART_CR1_TXEIE;
	}

	/* Enable interrupts in NVIC */
	NVIC_EnableIRQ(hal_uart_get_interrupt_source(uart_inst));

	return ret;
}

error_e hal_uart_retrieve(hal_uart_uart_num_e uart_num,
					 	  uint8_t *buf,
						  uint8_t len)
{
	USART_TypeDef *uart_inst = hal_uart_inst[uart_num];
	error_e ret;

	/* Disable interrupts from the UART instance */
	NVIC_DisableIRQ(hal_uart_get_interrupt_source(uart_inst));

	/* Ensure the interrupt is disabled */
	__DSB();

	ret = hal_uart_circ_buff_get_data(uart_num, HAL_UART_CIRC_BUFF_DIR_RX, buf, len);

	/* Enable interrupts from the UART instance */
	NVIC_EnableIRQ(hal_uart_get_interrupt_source(uart_inst));

	return ret;
}

void hal_uart_flush_buffer(hal_uart_uart_num_e uart_num)
{
	hal_uart_init_circular_buffer(uart_num);
}

static void hal_uart_enable_clk(USART_TypeDef *uart_inst)
{
	/* By default, PCLK1 (or PCLK2 in the case of USART1) are the clock source for
	 * every UART/USART/LPUART. This is the desired configuration */
	if(uart_inst == LPUART1)

		RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN_Msk;

	else if(uart_inst == USART1)

		RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;

	else if(uart_inst == USART2)
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN_Msk;

//		RCC->CCIPR &= ~RCC_CCIPR_USART2SEL_1;
//		RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;
	}
	else if(uart_inst == USART3)

		RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN_Msk;

	else if(uart_inst == UART4)

		RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN_Msk;

	else if(uart_inst == UART5)

		RCC->APB1ENR1 |= RCC_APB1ENR1_UART5EN_Msk;
}

static IRQn_Type hal_uart_get_interrupt_source(USART_TypeDef *uart_inst)
{
	if(uart_inst == LPUART1)

		return LPUART1_IRQn;

	else if(uart_inst == USART1)

		return USART1_IRQn;

	else if(uart_inst == USART2)

		return USART2_IRQn;

	else if(uart_inst == USART3)

		return USART3_IRQn;

	else if(uart_inst == UART4)

		return UART4_IRQn;

	else

		return UART5_IRQn;
}

static void hal_uart_init_circular_buffer(hal_uart_uart_num_e uart_num)
{
	for(hal_uart_circ_buff_dir_e circ_buff_dir = 0;
		circ_buff_dir < HAL_UART_CIRC_BUFF_DIR_MAX;
		circ_buff_dir++)
	{
		hal_uart_circ_buff[uart_num][circ_buff_dir].in_ptr = 0;
		hal_uart_circ_buff[uart_num][circ_buff_dir].out_ptr = 0;
	}
}

static error_e hal_uart_set_baudrate(USART_TypeDef *uart_inst,
									 uint32_t clk_freq_hz,
									 uint32_t baudrate)
{
	uint32_t usartdiv;

	/* Oversampling must be 16, otherwise the register should be configured
	 * differently */

	usartdiv = clk_freq_hz / baudrate;

	uart_inst->BRR = usartdiv;

	return ERROR_NONE;
}

static error_e hal_uart_circ_buff_put_data(hal_uart_uart_num_e uart_num,
										   hal_uart_circ_buff_dir_e dir,
										   uint8_t *buf,
										   uint8_t len)
{
	uint16_t i;
	uint16_t available_size;

	/* Calculate available room in the circular buffer */
	if(hal_uart_circ_buff[uart_num][dir].in_ptr
			>= hal_uart_circ_buff[uart_num][dir].out_ptr)
	{
		available_size =
			HAL_UART_BUFFER_DEPTH
			- hal_uart_circ_buff[uart_num][dir].in_ptr
			+ hal_uart_circ_buff[uart_num][dir].out_ptr;

	}
	else
	{
		available_size =
			hal_uart_circ_buff[uart_num][dir].out_ptr
			- hal_uart_circ_buff[uart_num][dir].in_ptr;
	}

	/* Is there enough room? */
	if(available_size < len)
		/* Not enough room! */
		return ERROR_UART_BUFFER_FULL;

	/* Add data to the buffer */
	if(hal_uart_circ_buff[uart_num][dir].in_ptr + len
		>= HAL_UART_BUFFER_DEPTH)
	{
		for(i = hal_uart_circ_buff[uart_num][dir].in_ptr;
			i < HAL_UART_BUFFER_DEPTH;
			i++)

			hal_uart_circ_buff[uart_num][dir].buffer[i] =
				buf[i - hal_uart_circ_buff[uart_num][dir].in_ptr];

		for(i = 0;
			i < len + hal_uart_circ_buff[uart_num][dir].in_ptr - HAL_UART_BUFFER_DEPTH;
			i++)

			hal_uart_circ_buff[uart_num][dir].buffer[i] =
				buf[len + hal_uart_circ_buff[uart_num][dir].in_ptr - HAL_UART_BUFFER_DEPTH + i - 1];

		/* Update pointer */
		hal_uart_circ_buff[uart_num][dir].in_ptr +=
			(len - HAL_UART_BUFFER_DEPTH);
	}
	else
	{
		for(i = hal_uart_circ_buff[uart_num][dir].in_ptr;
			i < hal_uart_circ_buff[uart_num][dir].in_ptr + len;
			i++)

			hal_uart_circ_buff[uart_num][dir].buffer[i] =
				buf[i - hal_uart_circ_buff[uart_num][dir].in_ptr];

		/* Update pointer */
		hal_uart_circ_buff[uart_num][dir].in_ptr += len;
	}
	return ERROR_NONE;
}

static error_e hal_uart_circ_buff_get_data(hal_uart_uart_num_e uart_num,
										   hal_uart_circ_buff_dir_e dir,
										   uint8_t *buf,
										   uint8_t len)
{
	uint16_t i;
	uint16_t available_data;

	/* Calculate available data in the circular buffer */
	if(hal_uart_circ_buff[uart_num][dir].out_ptr
			> hal_uart_circ_buff[uart_num][dir].in_ptr)

		available_data =
			HAL_UART_BUFFER_DEPTH
			- hal_uart_circ_buff[uart_num][dir].out_ptr
			+ hal_uart_circ_buff[uart_num][dir].in_ptr;

	else if(hal_uart_circ_buff[uart_num][dir].out_ptr
			< hal_uart_circ_buff[uart_num][dir].in_ptr)

		available_data =
			hal_uart_circ_buff[uart_num][dir].in_ptr
			- hal_uart_circ_buff[uart_num][dir].out_ptr;

	else

		available_data = 0;

	/* Is there enough data? */
	if(available_data < len)

		return ERROR_UART_BUFFER_EMPTY;

	/* Get data */
	if(hal_uart_circ_buff[uart_num][dir].out_ptr + len
		>= HAL_UART_BUFFER_DEPTH)
	{
		for(i = hal_uart_circ_buff[uart_num][dir].out_ptr;
			i < HAL_UART_BUFFER_DEPTH;
			i++)

			buf[i - hal_uart_circ_buff[uart_num][dir].out_ptr] =
				hal_uart_circ_buff[uart_num][dir].buffer[i];

		for(i = 0;
			i < len + hal_uart_circ_buff[uart_num][dir].out_ptr - HAL_UART_BUFFER_DEPTH;
			i++)

			buf[len + hal_uart_circ_buff[uart_num][dir].out_ptr - HAL_UART_BUFFER_DEPTH + i] =
				hal_uart_circ_buff[uart_num][dir].buffer[i];

		/* Update pointer */
		hal_uart_circ_buff[uart_num][dir].out_ptr +=
			(len - HAL_UART_BUFFER_DEPTH);
	}
	else
	{
		for(i = hal_uart_circ_buff[uart_num][dir].out_ptr;
			i < hal_uart_circ_buff[uart_num][dir].out_ptr + len;
			i++)

			buf[i - hal_uart_circ_buff[uart_num][dir].out_ptr] =
				hal_uart_circ_buff[uart_num][dir].buffer[i];

		/* Update pointer */
		hal_uart_circ_buff[uart_num][dir].out_ptr += len;
	}

	return ERROR_NONE;
}

static void hal_uart_interrupt_handler(hal_uart_uart_num_e uart_num)
{
	uint8_t data;
	USART_TypeDef *uart_inst = hal_uart_inst[uart_num];

	if(uart_num >= HAL_UART_UART_MAX)

		return;

	if((uart_inst->ISR & USART_ISR_TC) == USART_ISR_TC
		&& (uart_inst->CR1 & USART_CR1_TCIE) == USART_CR1_TCIE)
	{
		/* Transfer complete. Disable TCIE */
		uart_inst->CR1 &= ~(USART_CR1_TCIE);
	}
	else if((uart_inst->ISR & USART_ISR_TXE) == USART_ISR_TXE
		&& (uart_inst->CR1 & USART_CR1_TXEIE) == USART_CR1_TXEIE)
	{
		/* Transmit buffer empty is what triggered the interrupt */

		if(hal_uart_circ_buff_get_data(uart_num,
									   HAL_UART_CIRC_BUFF_DIR_TX,
									   &data,
									   1)
					== ERROR_NONE)
		{
			/* There's still data to be sent */
			uart_inst->TDR = data;
		}
		else
		{
			/* No more data to be sent. Disable TXE interrupt */
			uart_inst->CR1 &= ~(USART_CR1_TXEIE);

			/* Enable TC interrupt */
			uart_inst->CR1 |= USART_CR1_TCIE;
		}
	}
	else if((uart_inst->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		data = uart_inst->RDR;

		/* If the data can't enter the buffer -> problem */
		hal_uart_circ_buff_put_data(uart_num,
									HAL_UART_CIRC_BUFF_DIR_RX,
									&data,
									1);
	}
}

void USART1_IRQHandler(void)
{

}

void USART2_IRQHandler(void)
{
	hal_uart_interrupt_handler(HAL_UART_USART_2);
}

void USART3_IRQHandler(void)
{

}

void UART4_IRQHandler(void)
{

}

void UART5_IRQHandler(void)
{

}

USART_TypeDef *hal_uart_inst[HAL_UART_UART_MAX] =
{
		USART2
};
