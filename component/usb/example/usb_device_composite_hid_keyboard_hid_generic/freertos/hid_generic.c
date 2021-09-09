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

#include "usb_device_class.h"
#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "hid_generic.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static usb_device_composite_struct_t *s_UsbDeviceComposite;
static usb_device_hid_generic_struct_t s_UsbDeviceHidGeneric;

/*******************************************************************************
 * Code
 ******************************************************************************/

usb_status_t USB_DeviceHidGenericCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            break;
        case kUSB_DeviceHidEventRecvResponse:
            if (s_UsbDeviceComposite->attach)
            {
                USB_DeviceHidSend(s_UsbDeviceComposite->hidGenericHandle, USB_HID_GENERIC_ENDPOINT_IN,
                                  (uint8_t *)s_UsbDeviceHidGeneric.buffer[s_UsbDeviceHidGeneric.bufferIndex],
                                  USB_HID_GENERIC_OUT_BUFFER_LENGTH);
                s_UsbDeviceHidGeneric.bufferIndex ^= 1U;
                return USB_DeviceHidRecv(s_UsbDeviceComposite->hidGenericHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                                         (uint8_t *)s_UsbDeviceHidGeneric.buffer[s_UsbDeviceHidGeneric.bufferIndex],
                                         USB_HID_GENERIC_OUT_BUFFER_LENGTH);
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
    }

    return error;
}

usb_status_t USB_DeviceHidGenericSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        return USB_DeviceHidRecv(handle, USB_HID_GENERIC_ENDPOINT_OUT,
                                 (uint8_t *)s_UsbDeviceHidGeneric.buffer[s_UsbDeviceHidGeneric.bufferIndex],
                                 USB_HID_GENERIC_OUT_BUFFER_LENGTH);
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidGenericSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (USB_HID_GENERIC_INTERFACE_INDEX == interface)
    {
        return USB_DeviceHidRecv(handle, USB_HID_GENERIC_ENDPOINT_OUT,
                                 (uint8_t *)s_UsbDeviceHidGeneric.buffer[s_UsbDeviceHidGeneric.bufferIndex],
                                 USB_HID_GENERIC_OUT_BUFFER_LENGTH);
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidGenericInit(usb_device_composite_struct_t *deviceComposite)
{
    s_UsbDeviceComposite              = deviceComposite;
    s_UsbDeviceHidGeneric.bufferIndex = 0U;
    s_UsbDeviceHidGeneric.idleRate    = 125U;
    return kStatus_USB_Success;
}
