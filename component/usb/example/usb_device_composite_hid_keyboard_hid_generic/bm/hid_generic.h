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

extern usb_status_t USB_DeviceHidGenericInit(usb_device_composite_struct_t *deviceComposite);
extern usb_status_t USB_DeviceHidGenericCallback(class_handle_t handle, uint32_t event, void *param);
extern usb_status_t USB_DeviceHidGenericSetConfigure(class_handle_t handle, uint8_t configure);
extern usb_status_t USB_DeviceHidGenericSetInterface(class_handle_t handle,
                                                     uint8_t interface,
                                                     uint8_t alternateSetting);

#endif /* __USB_DEVICE_HID_GENERIC_H__ */
