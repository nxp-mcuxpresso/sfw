/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "keyboard.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceHidKeyboardAction(void);
static usb_status_t USB_DeviceHidKeyboardInterruptIn(usb_device_handle handle,
                                                     usb_device_endpoint_callback_message_struct_t *message,
                                                     void *callbackParam);
/*******************************************************************************
 * Variables
 ******************************************************************************/

static usb_device_composite_struct_t *s_UsbDeviceComposite;
static usb_device_hid_keyboard_struct_t s_UsbDeviceHidKeyboard;
/*******************************************************************************
 * Code
 ******************************************************************************/

static usb_status_t USB_DeviceHidKeyboardAction(void)
{
    static int8_t x = 0U;
    enum
    {
        DOWN,
        UP
    };
    static uint8_t dir = DOWN;

    switch (dir)
    {
        case DOWN:
            s_UsbDeviceHidKeyboard.buffer[2] = KEY_PAGEDOWN;
            x++;
            if (x > 10U)
            {
                dir++;
            }
            break;
        case UP:
            s_UsbDeviceHidKeyboard.buffer[2] = KEY_PAGEUP;
            x--;
            if (x < 1U)
            {
                dir = DOWN;
            }
            break;
    }
    return USB_DeviceSendRequest(s_UsbDeviceComposite->deviceHandle, USB_HID_KEYBOARD_ENDPOINT_IN,
                                 s_UsbDeviceHidKeyboard.buffer, USB_HID_KEYBOARD_REPORT_LENGTH);
}

static usb_status_t USB_DeviceHidKeyboardInterruptIn(usb_device_handle handle,
                                                     usb_device_endpoint_callback_message_struct_t *message,
                                                     void *callbackParam)
{
    if (s_UsbDeviceComposite->attach)
    {
        return USB_DeviceHidKeyboardAction();
    }

    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidKeyboardSetConfigure(usb_device_handle handle, uint8_t configure)
{
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;

    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        epCallback.callbackFn    = USB_DeviceHidKeyboardInterruptIn;
        epCallback.callbackParam = handle;

        epInitStruct.zlt          = 0U;
        epInitStruct.transferType = USB_ENDPOINT_INTERRUPT;
        epInitStruct.endpointAddress =
            USB_HID_KEYBOARD_ENDPOINT_IN | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
        if (USB_SPEED_HIGH == s_UsbDeviceComposite->speed)
        {
            epInitStruct.maxPacketSize = HS_HID_KEYBOARD_INTERRUPT_IN_PACKET_SIZE;
        }
        else
        {
            epInitStruct.maxPacketSize = FS_HID_KEYBOARD_INTERRUPT_IN_PACKET_SIZE;
        }

        USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);

        return USB_DeviceHidKeyboardAction(); /* run the cursor movement code */
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidKeyboardClassRequest(usb_device_handle handle,
                                               usb_setup_struct_t *setup,
                                               uint8_t **buffer,
                                               uint32_t *length)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    if (setup->wIndex != USB_HID_KEYBOARD_INTERFACE_INDEX)
    {
        return error;
    }

    switch (setup->bRequest)
    {
        case USB_DEVICE_HID_REQUEST_GET_REPORT:
            break;
        case USB_DEVICE_HID_REQUEST_GET_IDLE:
            break;
        case USB_DEVICE_HID_REQUEST_GET_PROTOCOL:
            break;
        case USB_DEVICE_HID_REQUEST_SET_REPORT:
            break;
        case USB_DEVICE_HID_REQUEST_SET_IDLE:
            error                           = kStatus_USB_Success;
            s_UsbDeviceHidKeyboard.idleRate = 125U;
            break;
        case USB_DEVICE_HID_REQUEST_SET_PROTOCOL:
            break;
    }

    return error;
}

usb_status_t USB_DeviceHidKeyboardInit(usb_device_composite_struct_t *deviceComposite)
{
    s_UsbDeviceComposite = deviceComposite;
    return kStatus_USB_Success;
}
