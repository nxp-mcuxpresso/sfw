/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "fsl_common.h"
#include "soft_timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile uint32_t g_SoftTimerCount;
static volatile uint32_t g_SetTimer;

/*******************************************************************************
 * Code
 ******************************************************************************/

void PD_DemoSoftTimer1msProcess(void)
{
    g_SoftTimerCount++;
    if (g_SetTimer > 0)
    {
        g_SetTimer--;
    }
}

void PD_DemoSoftTimer_init(void)
{
    g_SoftTimerCount = 0;
}

void PD_DemoSoftTimer_deinit(void)
{
}

uint32_t PD_DemoSoftTimer_msGet(void)
{
    return g_SoftTimerCount;
}

uint32_t PD_DemoSoftTimer_getInterval(uint32_t startTime)
{
    if (g_SoftTimerCount >= startTime)
    {
        return (g_SoftTimerCount - startTime);
    }
    else
    {
        return (0xFFFFFFFFu - (startTime - g_SoftTimerCount));
    }
}

void PD_DemoSoftTimer_msSleep(uint32_t delay)
{
    uint32_t tmp = g_SoftTimerCount;
    uint32_t interval;

    do
    {
        if (g_SoftTimerCount >= tmp)
        {
            interval = g_SoftTimerCount - tmp;
        }
        else
        {
            interval = 0xFFFFFFFFu - (tmp - g_SoftTimerCount);
        }
    } while (interval < delay);
}
