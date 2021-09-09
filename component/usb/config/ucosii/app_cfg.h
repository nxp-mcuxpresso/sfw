/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_CFG_H_
#define _APP_CFG_H_

/* Timer task priority */
#define OS_TASK_TMR_PRIO (OS_LOWEST_PRIO - 2u)

/* Define exception stack size */
#define OS_CPU_EXCEPT_STK_SIZE (2000U)

#endif /* _APP_CFG_H_ */
