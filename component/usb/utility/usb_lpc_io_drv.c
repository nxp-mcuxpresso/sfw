/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 0)
#include "fsl_gint.h"
#endif
#include "usb_io.h"
#include "fsl_gpio.h"

#ifdef SYSCON_STARTERP_GINT0_MASK
#define STARTER STARTERP
#define SYSCON_STARTER_GINT0_MASK SYSCON_STARTERP_GINT0_MASK
#endif
#ifdef SYSCON_STARTERP_GINT1_MASK
#define SYSCON_STARTER_GINT1_MASK SYSCON_STARTERP_GINT1_MASK
#endif

void USB_GpioInit(uint8_t instance, uint8_t port, uint32_t pinMask, usb_gpio_callback_t callback)
{
#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 0)
    GINT_Type *instanceList[] = GINT_BASE_PTRS;
    /* Initialize instanceList[instance] */
    GINT_Init(instanceList[instance]);

    /* Setup instanceList[instance] for edge trigger, "OR" mode */
    GINT_SetCtrl(instanceList[instance], kGINT_CombineOr, kGINT_TrigEdge, (gint_cb_t)callback);

    /* Select pins & polarity for instanceList[instance] */
    GINT_ConfigPins(instanceList[instance], (gint_port_t)port, ~(pinMask), pinMask);
#endif
}

void USB_GpioInt(uint8_t instance, uint8_t enable)
{
#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 0)
    GINT_Type *instanceList[] = GINT_BASE_PTRS;
    if (enable)
    {
#if !(defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
        if (0 == instance)
        {
            SYSCON->STARTER[0] |= SYSCON_STARTER_GINT0_MASK;
        }
        else if (1 == instance)
        {
            SYSCON->STARTER[0] |= SYSCON_STARTER_GINT1_MASK;
        }
        else
        {
        }
#endif
        GINT_EnableCallback(instanceList[instance]);
    }
    else
    {
        GINT_DisableCallback(instanceList[instance]);
    }
#endif
}

void USB_GpioInterruptInit(uint8_t instance, uint8_t port, uint32_t pin, uint8_t trrigger, usb_gpio_callback_t callback)
{
#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 0)
    uint8_t trig;
    GINT_Type *instanceList[] = GINT_BASE_PTRS;
    /* Initialize instanceList[instance] */
    GINT_Init(instanceList[instance]);

    switch (trrigger)
    {
        case kUSB_GpioInterruptLogicOne:
        case kUSB_GpioInterruptLogicZero:
            trig = kGINT_TrigLevel;
            break;

        default:
            trig = kGINT_TrigEdge;
            break;
    }
    /* Setup instanceList[instance] for edge trigger, "OR" mode */
    GINT_SetCtrl(instanceList[instance], kGINT_CombineOr, (gint_trig_t)trig, (gint_cb_t)callback);

    /* Select pins & polarity for instanceList[instance] */
    GINT_ConfigPins(instanceList[instance], (gint_port_t)port, ~(0x00000001 << pin), (0x00000001 << pin));
#endif
}

void USB_GpioInterruptEnable(uint8_t instance, uint8_t port, uint32_t pin, uint8_t enable)
{
#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 0)
    GINT_Type *instanceList[] = GINT_BASE_PTRS;
    if (enable)
    {
#if !(defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
        if (0 == instance)
        {
            SYSCON->STARTER[0] |= SYSCON_STARTER_GINT0_MASK;
        }
        else if (1 == instance)
        {
            SYSCON->STARTER[0] |= SYSCON_STARTER_GINT1_MASK;
        }
        else
        {
        }
#endif
        GINT_EnableCallback(instanceList[instance]);
    }
    else
    {
        GINT_DisableCallback(instanceList[instance]);
    }
#endif
}

void USB_GpioOutputInit(uint8_t instance, uint32_t port, uint32_t pin, uint8_t defaultValue)
{
    GPIO_Type *instanceList[]    = GPIO_BASE_PTRS;
    gpio_pin_config_t pin_config = {
        kGPIO_DigitalOutput,
        defaultValue,
    };

    GPIO_PinInit(instanceList[instance], port, pin, &pin_config);
}

void USB_GpioOutputWritePin(uint8_t instance, uint32_t port, uint32_t pin, uint8_t output)
{
    GPIO_Type *instanceList[] = GPIO_BASE_PTRS;

    GPIO_PinWrite(instanceList[instance], port, pin, output);
}

void USB_GpioInputInit(uint8_t instance, uint32_t port, uint32_t pin)
{
    GPIO_Type *instanceList[]    = GPIO_BASE_PTRS;
    gpio_pin_config_t pin_config = {
        kGPIO_DigitalInput,
        0,
    };

    GPIO_PinInit(instanceList[0], port, pin, &pin_config);
}

uint8_t USB_GpioInputReadPin(uint8_t instance, uint32_t port, uint32_t pin)
{
    GPIO_Type *instanceList[] = GPIO_BASE_PTRS;
    return GPIO_PinRead(instanceList[0], port, pin);
}
