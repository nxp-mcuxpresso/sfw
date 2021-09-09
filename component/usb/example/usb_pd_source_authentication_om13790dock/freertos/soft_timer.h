/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOFT_TIMER_H__
#define __SOFT_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize sofTimer module.
 */
void PD_DemoSoftTimer_init(void);

/*
 * De-initialize sofTimer module.
 */
void PD_DemoSoftTimer_deinit(void);

/*
 * Get the elapsed time in milliseconds.
 */
uint32_t PD_DemoSoftTimer_msGet(void);

/*
 * Get the interval time in milliseconds from the start time.
 */
uint32_t PD_DemoSoftTimer_getInterval(uint32_t startTime);

/*
 * Sleep for specified time.
 */
void PD_DemoSoftTimer_msSleep(uint32_t delay);

#ifdef __cplusplus
}
#endif

#endif
