/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_COMPOSITE_H__
#define __USB_DEVICE_COMPOSITE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;
    class_handle_t hidKeyboardHandle;
    class_handle_t hidGenericHandle;
    uint8_t speed;
    uint8_t attach;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_COMPOSITE_INTERFACE_COUNT];
} usb_device_composite_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* __USB_DEVICE_COMPOSITE_H__ */
