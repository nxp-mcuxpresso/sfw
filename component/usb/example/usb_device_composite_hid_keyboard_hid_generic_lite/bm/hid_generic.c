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

#include "hid_generic.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceHidGenericInterruptIn(usb_device_handle handle,
                                                    usb_device_endpoint_callback_message_struct_t *message,
                                                    void *callbackParam);
static usb_status_t USB_DeviceHidGenericInterruptOut(usb_device_handle handle,
                                                     usb_device_endpoint_callback_message_struct_t *message,
                                                     void *callbackParam);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static usb_device_composite_struct_t *s_UsbDeviceComposite;
static usb_device_hid_generic_struct_t s_UsbDeviceHidGeneric;

/*******************************************************************************
 * Code
 ******************************************************************************/

static usb_status_t USB_DeviceHidGenericInterruptIn(usb_device_handle handle,
                                                    usb_device_endpoint_callback_message_struct_t *message,
                                                    void *callbackParam)
{
    if (s_UsbDeviceComposite->attach)
    {
    }

    return kStatus_USB_Error;
}

static usb_status_t USB_DeviceHidGenericInterruptOut(usb_device_handle handle,
                                                     usb_device_endpoint_callback_message_struct_t *message,
                                                     void *callbackParam)
{
    if (s_UsbDeviceComposite->attach)
    {
        USB_DeviceSendRequest(s_UsbDeviceComposite->deviceHandle, USB_HID_GENERIC_ENDPOINT_IN,
                              (uint8_t *)s_UsbDeviceHidGeneric.buffer[s_UsbDeviceHidGeneric.bufferIndex],
                              USB_HID_GENERIC_OUT_BUFFER_LENGTH);
        s_UsbDeviceHidGeneric.bufferIndex ^= 1U;
        return USB_DeviceRecvRequest(s_UsbDeviceComposite->deviceHandle, USB_HID_GENERIC_ENDPOINT_OUT,
                                     (uint8_t *)s_UsbDeviceHidGeneric.buffer[s_UsbDeviceHidGeneric.bufferIndex],
                                     USB_HID_GENERIC_OUT_BUFFER_LENGTH);
    }

    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidGenericClassRequest(usb_device_handle handle,
                                              usb_setup_struct_t *setup,
                                              uint8_t **buffer,
                                              uint32_t *length)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    if (setup->wIndex != USB_HID_GENERIC_INTERFACE_INDEX)
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
            error                          = kStatus_USB_Success;
            s_UsbDeviceHidGeneric.idleRate = 125U;
            break;
        case USB_DEVICE_HID_REQUEST_SET_PROTOCOL:
            break;
    }

    return error;
}

usb_status_t USB_DeviceHidGenericSetConfigure(usb_device_handle handle, uint8_t configure)
{
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;

    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        epCallback.callbackFn    = USB_DeviceHidGenericInterruptIn;
        epCallback.callbackParam = handle;

        epInitStruct.zlt          = 0U;
        epInitStruct.transferType = USB_ENDPOINT_INTERRUPT;
        epInitStruct.endpointAddress =
            USB_HID_GENERIC_ENDPOINT_IN | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
        if (USB_SPEED_HIGH == s_UsbDeviceComposite->speed)
        {
            epInitStruct.maxPacketSize = HS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE;
        }
        else
        {
            epInitStruct.maxPacketSize = FS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE;
        }

        USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);

        epCallback.callbackFn    = USB_DeviceHidGenericInterruptOut;
        epCallback.callbackParam = handle;

        epInitStruct.zlt          = 0U;
        epInitStruct.transferType = USB_ENDPOINT_INTERRUPT;
        epInitStruct.endpointAddress =
            USB_HID_GENERIC_ENDPOINT_OUT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
        if (USB_SPEED_HIGH == s_UsbDeviceComposite->speed)
        {
            epInitStruct.maxPacketSize = HS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE;
        }
        else
        {
            epInitStruct.maxPacketSize = FS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE;
        }

        USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);

        return USB_DeviceRecvRequest(s_UsbDeviceComposite->deviceHandle, USB_HID_GENERIC_ENDPOINT_OUT,
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
