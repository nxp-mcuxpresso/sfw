/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_HID_JOYSTICK_H__
#define __USB_DEVICE_HID_JOYSTICK_H__

#if ((USB_DFU_JOYSTICK_COMPOSITE_EXAMPLE_V1 > 0U) || (USB_DFU_JOYSTICK_COMPOSITE_EXAMPLE_V2 > 0U))
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct _usb_device_hid_joystick_struct
{
    uint8_t buffer[USB_HID_JOYSTICK_REPORT_LENGTH];
    uint8_t idleRate;
} usb_device_hid_joystick_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

extern usb_status_t USB_DeviceHidJoystickSetConfigure(usb_device_handle handle, uint8_t configure);
extern usb_status_t USB_DeviceHidJoystickClassRequest(usb_device_handle handle,
                                                      usb_setup_struct_t *setup,
                                                      uint8_t **buffer,
                                                      uint32_t *length);
extern usb_status_t USB_DeviceHidJoystickInit(usb_device_composite_struct_t *deviceComposite);
extern usb_status_t USB_DeviceHidJoystickEndpointUnstall(usb_device_handle handle, uint8_t ep);
extern usb_status_t USB_DeviceHidJoystickEndpointStall(usb_device_handle handle, uint8_t ep);
#if defined(__cplusplus)
extern "C" {
#endif
#endif
#endif /* __USB_DEVICE_HID_JOYSTICK_H__ */
