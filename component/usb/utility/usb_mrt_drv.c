/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_mrt.h"
#include "usb_timer.h"

static uint32_t timerInterval;
static uint32_t mrtClock;
static usb_timer_callback_t timerCallback;

void MRT0_IRQHandler(void)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    /* Clear interrupt flag.*/
    MRT_ClearStatusFlags(instanceList[0], kMRT_Channel_0, kMRT_TimerInterruptFlag);
    timerCallback();
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void USB_TimerInit(uint8_t instance, uint32_t interval, uint32_t clock, usb_timer_callback_t callback)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    IRQn_Type instanceIrq[]  = MRT_IRQS;
    /* Structure of initialize MRT */
    mrt_config_t mrtConfig;

    /* mrtConfig.enableMultiTask = false; */
    MRT_GetDefaultConfig(&mrtConfig);

    /* Init mrt module */
    MRT_Init(instanceList[instance], &mrtConfig);

    /* Setup Channel 0 to be repeated */
    MRT_SetupChannelMode(instanceList[instance], kMRT_Channel_0, kMRT_RepeatMode);

    /* Enable timer interrupts for channel 0 */
    MRT_EnableInterrupts(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptEnable);

    timerInterval = interval;
    timerCallback = callback;
    mrtClock      = clock;

    /* Enable at the NVIC */
    EnableIRQ(instanceIrq[instance]);
}

void USB_TimerInt(uint8_t instance, uint8_t enable)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    if (enable)
    {
        /* Start channel 0 */
        MRT_StartTimer(instanceList[instance], kMRT_Channel_0, USEC_TO_COUNT(timerInterval, mrtClock));
    }
    else
    {
        /* Stop channel 0 */
        MRT_StopTimer(instanceList[instance], kMRT_Channel_0);
        /* Clear interrupt flag.*/
        MRT_ClearStatusFlags(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptFlag);
    }
}

/* return us tick count value */
uint32_t USB_TimerGetUsTickCount(uint8_t instance)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    uint32_t oneUsTick;
    uint32_t oneTickUs;
    uint32_t currentUsCount;

    if (mrtClock > 1000000)
    {
        oneUsTick = USEC_TO_COUNT(1, mrtClock);
        oneTickUs = 1;
    }
    else
    {
        oneUsTick = 1;
        oneTickUs = COUNT_TO_USEC(1, mrtClock);
    }

    /* counting down */
    currentUsCount = MRT_GetCurrentTimerCount(instanceList[instance], kMRT_Channel_0);
    /* counting up */
    currentUsCount = MRT_CHANNEL_TIMER_VALUE_MASK - currentUsCount;
    currentUsCount /= oneUsTick;
    currentUsCount *= oneTickUs;
    return currentUsCount;
}
