/*
 * hal_os.h
 *
 *  Created on: Feb 20, 2026
 *      Author: ricard
 */

#ifndef HAL_HAL_OS_HAL_OS_H_
#define HAL_HAL_OS_HAL_OS_H_

void hal_os_init(void);
void hal_os_start(void);
void hal_os_fxn(void);

void hal_os_task_attach(void (*fxn)(void));


#endif /* HAL_HAL_OS_HAL_OS_H_ */
