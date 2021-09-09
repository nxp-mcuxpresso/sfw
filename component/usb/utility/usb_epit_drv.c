/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_epit.h"
#include "usb_timer.h"
#include "fsl_gpc.h"

static usb_timer_callback_t timerCallback;

void EPIT1_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    EPIT_ClearStatusFlags(EPIT1, kEPIT_OutputCompareFlag);
    timerCallback();
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void USB_TimerInit(uint8_t instance, uint32_t interval, uint32_t clock, usb_timer_callback_t callback)
{
    EPIT_Type *instanceList[] = EPIT_BASE_PTRS;
    IRQn_Type instanceIrq[]   = EPIT_IRQS;
    /* Structure of initialize PIT */
    epit_config_t epitConfig;

    /*
     * lpitConfig.enableRunInDebug = false;
     * lpitConfig.enableRunInDoze = false;
     */
    EPIT_GetDefaultConfig(&epitConfig);
    epitConfig.clockSource = kEPIT_ClockSource_LowFreq; /* 32K RTC OSC */

    /* Init lpit module */
    EPIT_Init(instanceList[instance], &epitConfig);

    EPIT_SetOutputCompareValue(EPIT1, 0);

    /* Set timer period for channel 0 */
    EPIT_SetTimerPeriod(instanceList[instance], 31U); // Set the interrupt frequency to 1ms

    /* Enable timer interrupts for channel 0 */
    EPIT_EnableInterrupts(instanceList[instance], kEPIT_OutputCompareInterruptEnable);

    timerCallback = callback;

    /* Enable at the NVIC */
    EnableIRQ(instanceIrq[instance]);
}

void USB_TimerInt(uint8_t instance, uint8_t enable)
{
    EPIT_Type *instanceList[] = EPIT_BASE_PTRS;
    if (enable)
    {
        /* Start counting */
        EPIT_StartTimer(instanceList[instance]);
    }
    else
    {
        EPIT_StopTimer(instanceList[instance]);
    }
}
