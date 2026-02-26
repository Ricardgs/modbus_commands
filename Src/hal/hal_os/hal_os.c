/*
 * hal_os.c
 *
 *  Created on: Feb 20, 2026
 *      Author: ricard
 */

#include <stdint.h>
#include <stdlib.h>
#include "hal_os.h"


#define HAL_OS_MAX_TASKS	30

static void (*hal_os_tasks[HAL_OS_MAX_TASKS])(void);
static uint8_t hal_os_task_cnt;

void hal_os_init(void)
{
	hal_os_task_cnt = 0;
}

void hal_os_start(void)
{
	/* Nothing to be done here */
}

void hal_os_fxn(void)
{
	for(uint8_t i = 0; i < hal_os_task_cnt; i++)

		hal_os_tasks[i]();
}

void hal_os_task_attach(void (*fxn)(void))
{
	if(fxn != NULL)

		hal_os_tasks[hal_os_task_cnt++] = fxn;
}
