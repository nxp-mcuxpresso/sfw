/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_hid.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include <stdio.h>
#include <stdlib.h>
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
extern void Init_Board_Audio(void);
extern usb_status_t USB_DeviceHidKeyboardAction(void);
extern char *SW_GetName(void);
extern void USB_AudioCodecTask(void);
extern void USB_DeviceAudioSpeakerStatusReset(void);
extern void USB_DeviceAudioRecorderStatusReset(void);
extern usb_status_t USB_DeviceAudioRecorderSetInterface(usb_device_handle handle,
                                                        uint8_t interface,
                                                        uint8_t alternateSetting);
extern usb_status_t USB_DeviceAudioSpeakerSetInterface(usb_device_handle handle,
                                                       uint8_t interface,
                                                       uint8_t alternateSetting);
extern void USB_AudioSpeakerResetTask(void);
extern usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand, uint32_t *length, uint8_t **buffer, uint8_t entityOrEndpoint);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Composite device structure. */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_device_composite_struct_t g_composite;
extern volatile bool g_ButtonPress;
extern usb_device_composite_struct_t *g_UsbDeviceComposite;
extern usb_device_composite_struct_t *g_deviceComposite;
extern uint8_t g_UsbDeviceInterface[USB_COMPOSITE_INTERFACE_COUNT];
extern uint32_t totalCount;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_SetupOutBuffer[8];
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle		  The USB device handle.
 * @param event 		  The USB device event type.
 * @param param 		  The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint8_t *temp8     = (uint8_t *)param;
    uint8_t count      = 0U;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            /* USB bus reset signal detected */
            /* Initialize the control IN and OUT pipes */
            for (count = 0U; count < USB_DEVICE_INTERFACE_COUNT; count++)
            {
                g_composite.currentInterfaceAlternateSetting[count] = 0U;
            }
            USB_DeviceControlPipeInit(handle);
            g_composite.attach               = 0U;
            g_composite.currentConfiguration = 0U;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceGetStatus(handle, kUSB_DeviceStatusSpeed, &g_composite.speed))
            {
                USB_DeviceSetSpeed(g_composite.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_composite.attach               = 0U;
                g_composite.currentConfiguration = 0U;
            }
            else if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
            {
                g_composite.attach               = 1U;
                g_composite.currentConfiguration = *temp8;
                USB_DeviceAudioUnifiedSetConfigure(handle, *temp8);
                USB_DeviceHidKeyboardSetConfigure(handle, *temp8);
                error = kStatus_USB_Success;
            }
            else
            {
                error = kStatus_USB_InvalidRequest;
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_composite.attach)
            {
                uint8_t interface        = (uint8_t)(*temp8);
                uint8_t alternateSetting = (uint8_t)g_UsbDeviceInterface[interface];
                if (USB_AUDIO_RECORDER_STREAM_INTERFACE_INDEX == interface)
                {
                    if (g_composite.audioUnified.currentInterfaceAlternateSetting[interface] != alternateSetting)
                    {
                        if (g_composite.audioUnified.currentInterfaceAlternateSetting[interface] != alternateSetting)
                        {
                            if (g_composite.audioUnified.currentInterfaceAlternateSetting[interface])
                            {
                                USB_DeviceDeinitEndpoint(
                                    g_composite.deviceHandle,
                                    USB_AUDIO_RECORDER_STREAM_ENDPOINT |
                                        (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
                            }
                            else
                            {
                                USB_DeviceAudioRecorderStatusReset();
                                USB_DeviceAudioRecorderSetInterface(handle, interface, alternateSetting);
                            }
                            g_composite.audioUnified.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        }
                    }
                }
                else if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_INDEX == interface)
                {
                    if (g_composite.audioUnified.currentInterfaceAlternateSetting[interface] != alternateSetting)
                    {
                        if (g_composite.audioUnified.currentInterfaceAlternateSetting[interface])
                        {
                            USB_DeviceDeinitEndpoint(g_composite.deviceHandle,
                                                     USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
                                                         (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
                        }
                        else
                        {
                            USB_DeviceAudioSpeakerStatusReset();
                            USB_DeviceAudioSpeakerSetInterface(handle, interface, alternateSetting);
                        }
                        g_composite.audioUnified.currentInterfaceAlternateSetting[interface] = alternateSetting;
                    }
                }
                error = kStatus_USB_Success;
            }
            break;
        default:
            break;
    }

    return error;
}

/*!
 * @brief Get the setup packet buffer.
 *
 * This function provides the buffer for setup packet.
 *
 * @param handle The USB device handle.
 * @param setupBuffer The pointer to the address of setup packet buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetSetupBuffer(usb_device_handle handle, usb_setup_struct_t **setupBuffer)
{
    static uint32_t compositeSetup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&compositeSetup;
    return kStatus_USB_Success;
}

/*!
 * @brief Get the vendor request data buffer.
 *
 * This function gets the data buffer for vendor request.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetVendorReceiveBuffer(usb_device_handle handle,
                                              usb_setup_struct_t *setup,
                                              uint32_t *length,
                                              uint8_t **buffer)
{
    return kStatus_USB_Error;
}

/*!
 * @brief Audio vendor specific callback function.
 *
 * This function handles the CDC vendor specific requests.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceProcessVendorRequest(usb_device_handle handle,
                                            usb_setup_struct_t *setup,
                                            uint32_t *length,
                                            uint8_t **buffer)
{
    return kStatus_USB_InvalidRequest;
}

/*!
 * @brief Configure remote wakeup feature.
 *
 * This function configures the remote wakeup feature.
 *
 * @param handle The USB device handle.
 * @param enable 1: enable, 0: disable.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceConfigureRemoteWakeup(usb_device_handle handle, uint8_t enable)
{
    return kStatus_USB_InvalidRequest;
}

/*!
 * @brief USB configure endpoint function.
 *
 * This function configure endpoint status.
 *
 * @param handle The USB device handle.
 * @param ep Endpoint address.
 * @param status A flag to indicate whether to stall the endpoint. 1: stall, 0: unstall.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    error              = USB_DeviceAudioUnifiedConfigureEndpointStatus(handle, ep, status);
    error              = USB_DeviceHidConfigureEndpointStatus(handle, ep, status);

    return error;
}

/*!
 * @brief Get the setup packet data buffer.
 *
 * This function gets the data buffer for setup packet.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetClassReceiveBuffer(usb_device_handle handle,
                                             usb_setup_struct_t *setup,
                                             uint32_t *length,
                                             uint8_t **buffer)
{
    if ((NULL == buffer) || ((*length) > sizeof(s_SetupOutBuffer)))
    {
        return kStatus_USB_InvalidRequest;
    }
    *buffer = s_SetupOutBuffer;
    return kStatus_USB_Success;
}

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceProcessClassRequest(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    /* Handle the audio class specific request. */
    uint8_t entityId      = (uint8_t)(setup->wIndex >> 0x08);
    uint32_t audioCommand = 0;
    if ((((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) != USB_REQUEST_TYPE_RECIPIENT_INTERFACE)) ||
        ((((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) == USB_REQUEST_TYPE_RECIPIENT_INTERFACE)) &&
         (USB_AUDIO_CONTROL_INTERFACE_INDEX == (setup->wIndex & 0xFFU))))
    {
        switch (entityId)
        {
            case USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID:
            case USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID:
                break;
            case USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID:
            case USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID:
                break;
            case USB_AUDIO_CONTROL_CLOCK_SOURCE_ID:
                if (((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN))
                {
                    switch (setup->wValue >> 8)
                    {
                        case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL:
                            if (setup->bRequest == USB_DEVICE_AUDIO_REQUEST_CUR)
                            {
                                audioCommand = USB_DEVICE_AUDIO_GET_CUR_SAM_FREQ_CONTROL;
                            }
                            else if (setup->bRequest == USB_DEVICE_AUDIO_REQUEST_RANGE)
                            {
                                audioCommand = USB_DEVICE_AUDIO_GET_RANGE_SAM_FREQ_CONTROL;
                            }
                            else
                            {
                            }
                            break;
                        case USB_DEVICE_AUDIO_CS_CLOCK_VALID_CONTROL:
                            audioCommand = USB_DEVICE_AUDIO_GET_CUR_CLOCK_VALID_CONTROL;
                            break;
                        default:
                            break;
                    }
                }
                else if (((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT))
                {
                    switch (setup->wValue >> 8)
                    {
                        case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL:
                            audioCommand = USB_DEVICE_AUDIO_SET_CUR_SAM_FREQ_CONTROL;
                            break;
                        case USB_DEVICE_AUDIO_CS_CLOCK_VALID_CONTROL:
                            audioCommand = USB_DEVICE_AUDIO_SET_CUR_CLOCK_VALID_CONTROL;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                }
                break;
            case USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID:
            case USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID:
                if (((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN))
                {
                    switch (setup->wValue >> 8)
                    {
                        case USB_DEVICE_AUDIO_FU_MUTE_CONTROL:
                            audioCommand = USB_DEVICE_AUDIO_GET_CUR_MUTE_CONTROL_AUDIO20;
                            break;
                        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL:
                            if (setup->bRequest == USB_DEVICE_AUDIO_REQUEST_CUR)
                            {
                                audioCommand = USB_DEVICE_AUDIO_GET_CUR_VOLUME_CONTROL_AUDIO20;
                            }
                            else if (setup->bRequest == USB_DEVICE_AUDIO_REQUEST_RANGE)
                            {
                                audioCommand = USB_DEVICE_AUDIO_GET_RANGE_VOLUME_CONTROL_AUDIO20;
                            }
                            else
                            {
                            }
                            break;
                        default:
                            break;
                    }
                }
                else if (((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT))
                {
                    switch (setup->wValue >> 8)
                    {
                        case USB_DEVICE_AUDIO_FU_MUTE_CONTROL:
                            audioCommand = USB_DEVICE_AUDIO_SET_CUR_MUTE_CONTROL_AUDIO20;
                            break;
                        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL:
                            if (setup->bRequest == USB_DEVICE_AUDIO_REQUEST_CUR)
                            {
                                audioCommand = USB_DEVICE_AUDIO_SET_CUR_VOLUME_CONTROL_AUDIO20;
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
            break;
        }
        error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    }
    else
    {
        if (USB_HID_KEYBOARD_INTERFACE_INDEX == (setup->wIndex & 0xFFU))
        {
            return USB_DeviceHidKeyboardClassRequest(handle, setup, buffer, length);
        }
        else
        {
            return error;
        }
    }

#else

    if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) != USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
    {
        return USB_DeviceAudioUnifiedClassRequestEndpRecipient(handle, setup, length, buffer);
    }
    else
    {
        if (USB_AUDIO_CONTROL_INTERFACE_INDEX == (setup->wIndex & 0xFFU))
        {
            return USB_DeviceAudioUnifiedClassRequestIntfRecipient(handle, setup, length, buffer);
        }
        else if (USB_HID_KEYBOARD_INTERFACE_INDEX == (setup->wIndex & 0xFFU))
        {
            return USB_DeviceHidKeyboardClassRequest(handle, setup, buffer, length);
        }
        else
        {
        }
    }
#endif
    return error;
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void APPInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    g_composite.speed        = USB_SPEED_FULL;
    g_composite.attach       = 0U;
    g_composite.deviceHandle = NULL;

    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_composite.deviceHandle))
    {
        usb_echo("USB device composite demo init failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device composite demo\r\n");
        usb_echo("Please Press  switch(%s) to mute/unmute device audio speaker.\r\n", SW_GetName());

        USB_DeviceAudioUnifiedInit(&g_composite);
        USB_DeviceHidKeyboardInit(&g_composite);
    }

    Init_Board_Audio();

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_composite.deviceHandle);
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    BOARD_InitHardware();

    APPInit();

    while (1)
    {
        USB_DeviceHidKeyboardAction();

        USB_AudioCodecTask();

        USB_AudioSpeakerResetTask();

#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_composite.deviceHandle);
#endif
    }
}
