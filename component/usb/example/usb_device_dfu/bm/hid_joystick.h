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

/*!
 * @brief device joystick initialization function.
 *
 * This function initializes the device joystick with the composite device class information.
 *
 * @param deviceComposite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
extern usb_status_t USB_DeviceHidJoystickInit(usb_device_composite_struct_t *deviceComposite);

/*!
 * @brief device Joystick callback function.
 *
 * This function handles the Hid class specific event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
extern usb_status_t USB_DeviceHidJoystickCallback(class_handle_t handle, uint32_t event, void *param);

/*!
 * @brief device joystick  set configuration function.
 *
 * This function sets configuration for HID joystick interface.
 *
 * @param handle The Hid class handle.
 * @param configure Hid class configured index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidJoystickSetConfigure(class_handle_t handle, uint8_t configure);

/*!
 * @brief device joystick set interface function.
 *
 * This function alternates joystick interface.
 *
 * @param handle The Hid class handle.
 * @param alternateSetting Hid class alternateSetting.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidJoystickSetInterface(class_handle_t handle,
                                                      uint8_t interface,
                                                      uint8_t alternateSetting);

#if defined(__cplusplus)
extern "C" {
#endif
#endif
#endif /* __USB_DEVICE_HID_JOYSTICK_H__ */
