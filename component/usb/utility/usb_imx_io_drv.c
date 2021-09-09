/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "usb_io.h"

void USB_GpioInterruptInit(uint8_t instance, uint8_t port, uint32_t pin, uint8_t trrigger, usb_gpio_callback_t callback)
{
    GPIO_Type *gpioList[]                  = GPIO_BASE_PTRS;
    gpio_interrupt_mode_t pinInterruptMode = kGPIO_NoIntmode;
    gpio_pin_config_t pin_config           = {kGPIO_DigitalInput, 1, kGPIO_IntLowLevel};

    switch (trrigger)
    {
        case kUSB_GpioInterruptLogicZero:
            pinInterruptMode = kGPIO_IntLowLevel;
            break;
        case kUSB_GpioInterruptLogicOne:
            pinInterruptMode = kGPIO_IntHighLevel;
            break;

        case kUSB_GpioInterruptRisingEdge:
            pinInterruptMode = kGPIO_IntRisingEdge;
            break;

        case kUSB_GpioInterruptFallingEdge:
            pinInterruptMode = kGPIO_IntFallingEdge;
            break;

        default:
            pinInterruptMode = kGPIO_IntRisingOrFallingEdge;
            break;
    }

    pin_config.interruptMode = pinInterruptMode;
    GPIO_PinInit(gpioList[instance], pin, &pin_config);
}

void USB_GpioInterruptEnable(uint8_t instance, uint8_t port, uint32_t pin, uint8_t enable)
{
    GPIO_Type *gpioList[] = GPIO_BASE_PTRS;

    if (enable)
    {
        GPIO_PortEnableInterrupts(gpioList[instance], 0x01u << pin);
    }
    else
    {
        GPIO_PortDisableInterrupts(gpioList[instance], 0x01u << pin);
    }
}

void USB_GpioOutputInit(uint8_t instance, uint32_t port, uint32_t pin, uint8_t defaultValue)
{
    GPIO_Type *instanceList[] = GPIO_BASE_PTRS;
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t pin_config = {kGPIO_DigitalOutput, defaultValue, kGPIO_NoIntmode};

    GPIO_PinInit(instanceList[instance], pin, &pin_config);
}

void USB_GpioOutputWritePin(uint8_t instance, uint32_t port, uint32_t pin, uint8_t output)
{
    GPIO_Type *instanceList[] = GPIO_BASE_PTRS;

    GPIO_PinWrite(instanceList[instance], pin, output);
}

void USB_GpioInputInit(uint8_t instance, uint32_t port, uint32_t pin)
{
    GPIO_Type *instanceList[]    = GPIO_BASE_PTRS;
    gpio_pin_config_t pin_config = {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};

    GPIO_PinInit(instanceList[instance], pin, &pin_config);
}

uint8_t USB_GpioInputReadPin(uint8_t instance, uint32_t port, uint32_t pin)
{
    GPIO_Type *instanceList[] = GPIO_BASE_PTRS;
    return GPIO_PinRead(instanceList[instance], pin);
}
