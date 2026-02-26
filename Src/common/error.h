/*
 * error.h
 *
 *  Created on: Jan 15, 2026
 *      Author: ricard
 */

#ifndef ERROR_H_
#define ERROR_H_

typedef enum
{
	ERROR_NONE,
	ERROR_INCORRECT_FREQ,
	ERROR_UART_BUFFER_FULL,
	ERROR_UART_BUFFER_EMPTY,
	ERROR_NON_EXISTENT_TIMER,
	ERROR_MODBUS_INEXISTENT_REGISTER,
	ERROR_MAX
} error_e;

#endif /* ERROR_H_ */
