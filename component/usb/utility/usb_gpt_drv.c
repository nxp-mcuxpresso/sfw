/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_gpt.h"
#include "usb_timer.h"

static usb_timer_callback_t timerCallback;

void GPT1_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(GPT1, kGPT_OutputCompare1Flag);
    timerCallback();
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
 exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void USB_TimerInit(uint8_t instance, uint32_t interval, uint32_t clock, usb_timer_callback_t callback)
{
    gpt_config_t gptConfig;
    GPT_Type *instanceList[] = GPT_BASE_PTRS;
    IRQn_Type instanceIrq[]  = GPT_IRQS;

    GPT_GetDefaultConfig(&gptConfig);
    /* Initialize GPT module */
    GPT_Init(instanceList[instance], &gptConfig);
    /* Set both GPT modules to 1 second duration */
    GPT_SetOutputCompareValue(instanceList[instance], kGPT_OutputCompare_Channel1, USEC_TO_COUNT(interval, clock));
    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(instanceList[instance], kGPT_OutputCompare1InterruptEnable);

    timerCallback = callback;
    /* Enable at the Interrupt */
    EnableIRQ(instanceIrq[instance]);
}

void USB_TimerInt(uint8_t instance, uint8_t enable)
{
    GPT_Type *instanceList[] = GPT_BASE_PTRS;
    if (enable)
    {
        /* Start Timer */
        GPT_StartTimer(instanceList[instance]);
    }
    else
    {
        GPT_StopTimer(instanceList[instance]);
    }
}
