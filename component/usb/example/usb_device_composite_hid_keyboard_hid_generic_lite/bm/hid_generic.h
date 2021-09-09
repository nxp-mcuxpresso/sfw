/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_HID_GENERIC_H__
#define __USB_DEVICE_HID_GENERIC_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _usb_device_hid_generic_struct
{
    uint32_t buffer[2][USB_HID_GENERIC_IN_BUFFER_LENGTH >> 2];
    uint8_t bufferIndex;
    uint8_t idleRate;
} usb_device_hid_generic_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

extern usb_status_t USB_DeviceHidGenericSetConfigure(usb_device_handle handle, uint8_t configure);
extern usb_status_t USB_DeviceHidGenericClassRequest(usb_device_handle handle,
                                                     usb_setup_struct_t *setup,
                                                     uint8_t **buffer,
                                                     uint32_t *length);
extern usb_status_t USB_DeviceHidGenericInit(usb_device_composite_struct_t *deviceComposite);

#endif /* __USB_DEVICE_HID_GENERIC_H__ */
