/*
 * status.h
 *
 *  Created on: Feb 15, 2026
 *      Author: ricard
 */

#ifndef COMMON_STATUS_H_
#define COMMON_STATUS_H_

typedef enum
{
	STATUS_NOT_INIT,
	STATUS_NOT_STARTED,
	STATUS_STARTED,
	STATUS_ERROR,
	STATUS_MAX
} status_e;

#endif /* COMMON_STATUS_H_ */
