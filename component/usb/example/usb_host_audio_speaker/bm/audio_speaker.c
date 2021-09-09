/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "audio_speaker.h"
#include "usb_host_audio.h"
#include "fsl_debug_console.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
void Audio_SendData(void);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#define MAX_ISO_PACKET_SIZE 1024U
#define NUMBER_OF_BUFFER 0x5U
#define NUMBER_OF_COMMAND_RETRY 0x3U
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* USB audio transfer Types string*/
static char *strTransferType[4] = {"Control", "Isochronous", "Bulk", "Interrupt"};
/* USB audio Sync Types string*/
static char *strSyncType[4] = {"No synchronization", "Asynchronous", "Adaptive", "Synchronous"};
/* USB audio Data Types string*/
static char *strDataType[4] = {"Data endpoint", "Feedback endpoint", "Implicit feedback", "Reserved"};
usb_device_handle g_audioDeviceHandle;
usb_host_interface_handle g_audioOutControlifHandle;
usb_host_interface_handle g_audioOutStreamifHandle;
audio_speraker_instance_t g_audio;
USB_RAM_ADDRESS_ALIGNMENT(USB_CACHE_LINESIZE) uint8_t g_wavBuff[MAX_ISO_PACKET_SIZE];
uint32_t g_packetSize;
extern const unsigned char wav_data[];
extern const uint32_t wav_size;
uint8_t g_hostCurVolume = 4U;
uint16_t g_deviceVolumeStep;
static uint8_t g_index                                        = 0U;
usb_audio_stream_format_type_desc_t *g_pFormatTypeDesc        = NULL;
usb_audio_2_0_stream_format_type_desc_t *g_pFormatTypeDesc_20 = NULL;
usb_audio_2_0_stream_spepific_as_intf_desc_t *g_generalDesc_20 = NULL;
usb_audio_stream_spepific_as_intf_desc_t *g_pAsItfDesc        = NULL;
usb_descriptor_endpoint_t *g_pIsoEndpDesc = NULL;
static uint16_t g_curVolume;
static uint16_t g_minVolume;
static uint16_t g_maxVolume;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout2_struct_t  g_volumeRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout3_struct_t  g_frequencyRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_curVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_curMute[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t minVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t maxVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t resVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t freq;
uint8_t *g_frequencyAllRang;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief usb host audio control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param    the host audio instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status         transfer result status.
 */
void Audio_ControlCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    audio_speraker_instance_t *audio_ptr = (audio_speraker_instance_t *)param;
    static uint32_t retryCount           = 0;

    if (status != kStatus_USB_Success)
    {
        retryCount++;
        if (kUSB_HostAudioRunWaitAudioSetCurSamplingFreq == audio_ptr->runWaitState)
        {
            usb_echo("audio speaker doest not support SamplingFreq request!\r\n");
            /* set idle, don't do retry */
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMinVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetMinVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunSetInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMaxVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetMaxVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetMaxVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetResVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetResVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetResVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetVolumeRang)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetVolumeRang request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunSetInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetCurrentVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetCurrentVolume;
        }

        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetSampleFrequencyRange)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetSampleFrequencyRange request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetSampleFrequencyRange;
        }
        else /* other states, don't do retry */
        {
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }

        if (kUSB_HostAudioRunIdle != audio_ptr->runState)
        {
            if (retryCount < NUMBER_OF_COMMAND_RETRY)
            {
                return;
            }
            else
            {
                audio_ptr->runState = kUSB_HostAudioRunIdle;
            }
        }
    }
    if (audio_ptr->runState == kUSB_HostAudioRunIdle)
    {
        retryCount = 0;
        if (audio_ptr->runWaitState == kUSB_HostAudioRunSetControlInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetVolumeRang)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetCurrentVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetSampleFrequencyRange;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetSampleFrequencyRange)
        {
            /*device support discrete frequency list, will get all the frequency */
            if(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0])) > 1U)
            {
                 audio_ptr->runState = kUSB_HostAudioRunAudioGetAllSampleFrequencyRange;
            }
            else
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
            }
            
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetAllSampleFrequencyRange)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMinVolume)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetMaxVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMaxVolume)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetResVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetResVolume)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel1Vol)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel2Vol;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioSetCurSamplingFreq)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel2Vol)
        {
            if (g_audio.deviceIsUsed == 0)
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioSetCurSamplingFreq;
            }
            else if (g_audio.deviceIsUsed == 1)
            {
                audio_ptr->runState = kUSB_HostAudioRunIdle;
            }
            else
            {
            }
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigMute)
        {
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }
        else
        {
        }
    }
}

/*!
 * @brief host audio  iso  out transfer callback.
 *
 * This function is used as callback function when call usb_host_audio_send .
 *
 * @param param    the host audio instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
void Audio_OutCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    audio_speraker_instance_t *audio_ptr = (audio_speraker_instance_t *)param;
    if (status == kStatus_USB_TransferCancel)
    {
        return;
    }
    g_audio.bufCount--;

    if (status == kStatus_USB_Success)
    {
        if (audio_ptr->devState == kStatus_DEV_Attached)
        {
            Audio_SendData();
        }
        else
        {
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }
    }
    else
    {
        if (audio_ptr->devState == kStatus_DEV_Attached)
        {
            Audio_SendData();
        }
    }
}

/*!
 * @brief prepare usb transfer data.
 */
static void USB_AudioSpeakerGetBuffer(uint8_t *buffer, uint32_t size)
{
    uint32_t resolution_size       = size >> 5U;
    static uint32_t audio_position = 0U;

    uint8_t k, j = 0U;

    for (k = 0U; k < 32U; k++)
    {
        for (j = 0U; j < resolution_size; j++)
        {
            buffer[j * 32 + k] = wav_data[audio_position];
        }
        audio_position++;
    }

    if (audio_position >= wav_size)
    {
        audio_position = 0U;
    }
}

/*!
 * @brief host audio iso out transfer send data function.
 *
 * This function is used to send iso data.
 *
 */
void Audio_SendData(void)
{
    usb_status_t status = kStatus_USB_Success;

#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
    while (g_audio.bufCount < 3U)
#endif
    {
        USB_AudioSpeakerGetBuffer(&g_wavBuff[g_packetSize * g_index], g_audio.sendPacketSize);
        status = USB_HostAudioStreamSend(g_audio.classHandle, (unsigned char *)&g_wavBuff[g_packetSize * g_index],
                                         g_audio.sendPacketSize, Audio_OutCallback, &g_audio);
        if (status != kStatus_USB_Success)
        {
            usb_echo("Error in USB_HostAudioStreamSend: %x\r\n", status);
            return;
        }
        g_index++;
        if (g_index == NUMBER_OF_BUFFER)
        {
            g_index = 0U;
        }
        g_audio.bufCount++;
    }
}

/*!
 * @brief host usb audio task function.
 *
 * This function implements the host audio action, it is used to create task.
 *
 * @param param  the host audio instance pointer.
 */
void USB_AudioTask(void *arg)
{
    usb_status_t status = kStatus_USB_Success;
    uint32_t bsamfreqtype_g_index;

    /* device state changes */
    if (g_audio.devState != g_audio.prevState)
    {
        g_audio.prevState = g_audio.devState;
        switch (g_audio.devState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached:
                g_audio.runState     = kUSB_HostAudioRunSetControlInterface;
                g_audio.deviceIsUsed = 0;
                g_audio.bufCount     = 0;
                g_hostCurVolume      = 4;
                USB_HostAudioInit(g_audio.deviceHandle, &g_audio.classHandle);
                usb_echo("USB audio attached\r\n");
                break;

            case kStatus_DEV_Detached:
                g_audio.devState = kStatus_DEV_Idle;
                g_audio.runState = kUSB_HostAudioRunIdle;
                USB_HostAudioDeinit(g_audio.deviceHandle, g_audio.classHandle);
                g_audio.classHandle = NULL;
                g_audio.bufCount    = 0;
                g_index = 0U;
                if(NULL != g_frequencyAllRang)
                {
                    free(g_frequencyAllRang);
                }
                usb_echo("audio speaker detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (g_audio.runState)
    {
        case kUSB_HostAudioRunIdle:
            break;

        case kUSB_HostAudioRunSetControlInterface:
            g_audio.runWaitState = kUSB_HostAudioRunSetControlInterface;
            g_audio.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioControlSetInterface(g_audio.classHandle, g_audio.controlIntfHandle, 0,
                                                 Audio_ControlCallback, &g_audio) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostAudioRunSetControlInterfaceDone:
            g_audio.runWaitState = kUSB_HostAudioRunWaitSetStreamInterface;
            g_audio.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioStreamSetInterface(g_audio.classHandle, g_audio.streamIntfHandle, 1, Audio_ControlCallback,
                                                &g_audio) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;

        case kUSB_HostAudioRunSetInterfaceDone:
            g_audio.runState     = kUSB_HostAudioRunIdle;
            {
                usb_audio_ctrl_common_header_desc_t *commonHeader;
                if (USB_HostAudioControlGetCurrentAltsettingSpecificDescriptors(
                g_audio.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                USB_DESC_SUBTYPE_AUDIO_CS_HEADER,
                (void *)&commonHeader) != kStatus_USB_Success)
                {
                    usb_echo("Get header descriptor error\r\n");
                    g_audio.runState     = kUSB_HostAudioRunIdle;
                }
                else
                {
                    g_audio.deviceAudioVersion = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)commonHeader->bcdcdc));
                    g_packetSize           = USB_HostAudioPacketSize(g_audio.classHandle, USB_ENDPOINT_ISOCHRONOUS, USB_OUT);
                    if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
                    {
                        usb_echo("AUDIO 1.0 device\r\n");
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                        g_audio.classHandle, USB_DESCRIPTOR_TYPE_ENDPOINT,
                        USB_AUDIO_EP_TYPE_DATA,
                        (void *)&g_pIsoEndpDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get data endpoint descriptor error\r\n");
                        }
                        
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                        g_audio.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                        USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE,
                        (void *)&g_pFormatTypeDesc ) != kStatus_USB_Success)
                        {
                            usb_echo("Get format descriptor error\r\n");
                        }
                        
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                        g_audio.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                        USB_AUDIO_DESC_SUBTYPE_AS_GENERAL,
                        (void *)&g_pAsItfDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get audio general descriptor error\r\n");
                        }
                        g_audio.sendPacketSize = (((((uint32_t)g_pFormatTypeDesc->tsamfreq[0][2]) << 16U) |
                                                   (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][1]) << 8U) |
                                                   (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][0]) << 0U)) /
                                                  1000U) *
                                                 g_pFormatTypeDesc->bnrchannels * (g_pFormatTypeDesc->bbitresolution / 8U);
                        g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetMinVolume;
                        g_audio.runState     = kUSB_HostAudioRunIdle;
                        if(kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U), (uint32_t)((1U << 8) | USB_AUDIO_CS_REQUEST_CODE_GET_MIN),
                                                                                    &minVol, sizeof(minVol),  Audio_ControlCallback,
                                                                                   &g_audio))
                        {
                            g_audio.runState = kUSB_HostAudioRunAudioDone;
                        }
                        usb_echo("AUDIO_GET_MIN_VOLUME\r\n");
                      
                    }
                    else if(AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
                    {
                        usb_echo("AUDIO 2.0 device\r\n");
                        
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                        g_audio.classHandle, USB_DESCRIPTOR_TYPE_ENDPOINT,
                        USB_AUDIO_EP_TYPE_DATA,
                        (void *)&g_pIsoEndpDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get data endpoint descriptor error\r\n");
                        }
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                        g_audio.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                        USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE_20,
                        (void *)&g_pFormatTypeDesc_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get format descriptor error\r\n");
                        }
                        
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                        g_audio.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                        USB_AUDIO_DESC_SUBTYPE_AS_GENERAL_20,
                        (void *)&g_generalDesc_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get audio general descriptor error\r\n");
                        }
     
                        g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetVolumeRang;
                        g_audio.runState     = kUSB_HostAudioRunIdle;
                        if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U), (uint32_t)((1U << 8) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20),
                                                                                    &g_volumeRang, sizeof(g_volumeRang),  Audio_ControlCallback,
                                                                                   &g_audio))
                        {
                            g_audio.runState = kUSB_HostAudioRunAudioDone;
                        }
                        usb_echo("AUDIO_GET_VOLUME_RANG\r\n");
                    }
                    else
                    {
                        usb_echo("unsupported audio device, current host only support audio1.0 and audio2.0\r\n");
                    }
                }
            }
            break;
        case kUSB_HostAudioRunAudioGetMaxVolume:
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetMaxVolume;
                g_audio.runState     = kUSB_HostAudioRunIdle;
                if(kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U), (uint32_t)((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_GET_MAX),
                                                                            &maxVol, sizeof(maxVol),  Audio_ControlCallback,
                                                                           &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }

            }
            break;

        case kUSB_HostAudioRunAudioGetResVolume:
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetResVolume;
                if(kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U), (uint32_t)((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_GET_RES),
                                                                            &resVol, sizeof(resVol),  Audio_ControlCallback,
                                                                            &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioGetCurrentVolume:
            if(AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U), ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20),
                                                                            &g_curVolume, sizeof(g_curVolume),  Audio_ControlCallback,
                                                                           &g_audio))


                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;

         case kUSB_HostAudioRunAudioGetSampleFrequencyRange:
            if(AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetSampleFrequencyRange;
                if (kStatus_USB_Success != USB_HostAudioGetSetClockSourceRequest(g_audio.classHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U), ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20 ),
                                                                            &g_frequencyRang, sizeof(g_frequencyRang),  Audio_ControlCallback,
                                                                           &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
         case kUSB_HostAudioRunAudioGetAllSampleFrequencyRange:
            if(AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                /*fresurency use layout 3 Parameter Block, so the lenght is 2+12*n*/
                g_frequencyAllRang = malloc(2U + 4U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0]))));
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetAllSampleFrequencyRange;
                if (kStatus_USB_Success != USB_HostAudioGetSetClockSourceRequest(g_audio.classHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U), ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20 ),
                                                                            &g_frequencyAllRang, sizeof(g_frequencyAllRang),  Audio_ControlCallback,
                                                                           &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioConfigChannel:
            g_audio.runState = kUSB_HostAudioRunIdle;
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_minVolume      = (uint16_t)(minVol[1] << 8U) | (minVol[0]);
                g_maxVolume      = (uint16_t)(maxVol[1] << 8U) | (maxVol[0]);
                g_deviceVolumeStep =
                    (int16_t)(((int16_t)(g_maxVolume) - (int16_t)(g_minVolume)) / (HOST_MAX_VOLUME - HOST_MIN_VOLUME));
                g_curVolume          = (int16_t)(g_minVolume + g_deviceVolumeStep * g_hostCurVolume);
                g_curVol[0]          = (int8_t)((uint16_t)(g_curVolume)&0x00FF);
                g_curVol[1]          = (int8_t)((uint16_t)(g_curVolume) >> 8U);
                g_curMute[0]         = 0U;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel;
                if(kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U), (uint32_t)((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR),
                                                                           &g_curMute, sizeof(g_curMute),  Audio_ControlCallback,
                                                                           &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
                }
            }
            else if(AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_minVolume      = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wMIN[0]));
                g_maxVolume      = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wMAX[0]));
                g_deviceVolumeStep = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wRES[0]));
                g_curVol[0]          = (int8_t)((uint16_t)(g_curVolume)&0x00FF);
                g_curVol[1]          = (int8_t)((uint16_t)(g_curVolume) >> 8U);
                g_curMute[0]         = 0U;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20),
                                                                           (void *)g_curMute, sizeof(g_curMute[0]),  Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
                }
            }
            else
            {
                /*TO DO*/
            }
            break;
        case kUSB_HostAudioRunAudioConfigMute:
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_curMute[0]         = !g_curMute[0];
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigMute;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR),
                                                                               (void *)g_curMute, sizeof(g_curMute[0]),  Audio_ControlCallback, &g_audio))

                {
                   usb_echo("config mute failed\n\r");
                }
                
            }
            else if((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_curMute[0]         = !g_curMute[0];
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigMute;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20),
                                                      (void *)g_curMute, sizeof(g_curMute[0]),  Audio_ControlCallback, &g_audio))
                {
                    usb_echo("config mute failed\n\r");
                }
            }
            else
            {
                    /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioConfigChannel1Vol:
            g_audio.runState     = kUSB_HostAudioRunIdle;
            g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel1Vol;
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {

                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR),
                                                                             (void *)g_curVol, sizeof(g_curVol),  Audio_ControlCallback, &g_audio))

                {
                    usb_echo("config volume failed\n\r");
                }
            }
            else if((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20),
                                                                           (void *)g_curVol, sizeof(g_curVol),  Audio_ControlCallback, &g_audio))
                {
                    usb_echo("config volume failed\n\r");
                }
            }
            else
            {
                    /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioConfigChannel2Vol:
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {

                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel2Vol;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 2U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR),
                                                                             (void *)g_curVol, sizeof(g_curVol),  Audio_ControlCallback, &g_audio))

                {
                    usb_echo("config channel 2 volume failed\n\r");
                }
            }
            else if((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel2Vol;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(g_audio.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 2U), ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20),
                                                                           (void *)g_curVol, sizeof(g_curVol),  Audio_ControlCallback, &g_audio))
                {
                    usb_echo("config channel 2 volume failed\n\r");
                }   
            }
            else
            {
                    /*TO DO*/
            }

            break;

        case kUSB_HostAudioRunAudioSetCurSamplingFreq:
            usb_echo("Audio Speaker device information:\r\n");
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_pFormatTypeDesc->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("   - Frequency device support      : %d Hz\n\r",
                             (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }

                usb_echo("   - Bit resolution : %d bits\n\r", g_pFormatTypeDesc->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_pFormatTypeDesc->bnrchannels);
                usb_echo("   - Transfer type : %s\n\r", strTransferType[(g_pIsoEndpDesc->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 2) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 4) & EP_TYPE_MASK]);

                usb_echo("This audio device supports play audio files with these properties:\r\n");
                usb_echo("   - Sample rate    :\r\n");
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_pFormatTypeDesc->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("                    : %d Hz\n\r",
                             (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }
                usb_echo("   - Sample size    : %d bits\n\r", g_pFormatTypeDesc->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_pFormatTypeDesc->bnrchannels);
                usb_echo("USB Speaker example will loop playback %dk_%dbit_%dch format audio.\r\n",
                         ((((uint32_t)g_pFormatTypeDesc->tsamfreq[0][2]) << 16U) |
                          (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][1]) << 8U) |
                          (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][0]) << 0U)) /
                             1000U,
                         g_pFormatTypeDesc->bbitresolution, g_pFormatTypeDesc->bnrchannels);

                freq = (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][2]) << 16U) |
                       (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][1]) << 8U) |
                       (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][0]) << 0U);
            }
            else if((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                uint8_t *frequencyArray;
                uint8_t *frequencyArrayStart;
                if( 1U < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0])))
                {
                    /*frequency array location in layout 3, offset 2 is the start address of frequency array*/
                    frequencyArray = (uint8_t *)(g_frequencyAllRang + 2U);
                    usb_echo("more than one frequenuy is support by device\r\n");
                    
                }
                else
                {
                    frequencyArray = (uint8_t *)&g_frequencyRang.wMIN[0];
                }
                frequencyArrayStart = frequencyArray;
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0]));
                bsamfreqtype_g_index++)
                {
                    /*frequency array location in layout 3*/
                    usb_echo("   - Frequency device support frequency rang is :MIN %d Hz,  MAX %d Hz, RES attributes %dHz, \n\r",
                            USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart)), USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 4U)), USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 8U)));
                    frequencyArrayStart += sizeof(usb_audio_2_0_layout3_struct_t);
                }
                usb_echo("   - Bit resolution : %d bits\n\r", g_pFormatTypeDesc_20->bBitResolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_generalDesc_20->bNrChannels);
                usb_echo("   - Transfer type : %s\n\r", strTransferType[(g_pIsoEndpDesc->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 2U) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 4U) & EP_TYPE_MASK]);
                
                usb_echo("USB Speaker example will loop playback %dk_%dbit_%d ch format audio.\r\n",
                         USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArray)) /1000U,
                         g_pFormatTypeDesc_20->bBitResolution,  g_generalDesc_20->bNrChannels);
                
                freq = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArray));
                g_audio.sendPacketSize = (g_pFormatTypeDesc_20->bBitResolution / 8U) * g_generalDesc_20->bNrChannels * (freq / 1000U);

            }
            else
            {
                    /*TO DO*/
            }
            g_audio.runState     = kUSB_HostAudioRunIdle;
            g_audio.runWaitState = kUSB_HostAudioRunWaitAudioSetCurSamplingFreq;
            if((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
            }

            if(g_audio.sendPacketSize > g_packetSize)
            {
                usb_echo("sendPacketSize is bigger than macpacket size");
                g_audio.sendPacketSize = g_packetSize;
            }
            g_packetSize = g_audio.sendPacketSize;
            if(AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                USB_HostAudioGetSetEndpointRequest(g_audio.classHandle, (uint32_t)((USB_AUDIO_EP_CS_SAMPING_FREQ_CONTROL << 8U)), 
                                                             ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR ), &freq,
                                                             sizeof(freq),  Audio_ControlCallback, &g_audio);
            }
            else if((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                USB_HostAudioGetSetClockSourceRequest(g_audio.classHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U), 
                                                             ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20 ), &freq,
                                                             sizeof(freq),  Audio_ControlCallback, &g_audio);
            }
            else
            {
                    /*TO DO*/
            }

            break;

        case kUSB_HostAudioRunAudioDone:
            g_index = 0U;
            while (g_audio.bufCount < 3U)
            {
                USB_AudioSpeakerGetBuffer(&g_wavBuff[g_packetSize * g_index], g_audio.sendPacketSize);
                g_audio.runState = kUSB_HostAudioRunIdle;
                if (USB_HostAudioStreamSend(g_audio.classHandle, (unsigned char *)&g_wavBuff[g_packetSize * g_index],
                                            g_audio.sendPacketSize, Audio_OutCallback, &g_audio) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostAudioStreamSend: %x\r\n", status);
                }
                g_index++;
                if (g_index == NUMBER_OF_BUFFER)
                {
                    g_index = 0U;
                }
                g_audio.bufCount++;
            }
            g_audio.deviceIsUsed = 1;
            break;

        default:
            break;
    }
}

/*!
 * @brief host usb audio mute request.
 *
 * This function implements the host audio mute request action, it is used to mute or unmute audio device.
 *
 */
void Audio_MuteRequest(void)
{
    if (g_audio.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Speaker is not connected\n\r");
        return;
    }

    if (g_audio.deviceIsUsed == 0)
    {
        usb_echo("  err: Audio Speaker is not Ready\n\r");
        return;
    }

    g_audio.runState = kUSB_HostAudioRunAudioConfigMute;
}

/*!
 * @brief host usb audio increase volume request.
 *
 * This function implements the host audio increase volume request action, it is used to increase the volume of audio
 * device.
 *
 * @param channel   the audio device channel number.
 */
void Audio_IncreaseVolumeRequest(uint8_t channel)
{
    static uint32_t i;
    uint8_t max_audio_channel;
    max_audio_channel = 2U;
    if (channel > max_audio_channel)
    {
        usb_echo("  err: Channel number larger than max channel\n\r");
        return;
    }
    if (g_audio.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Speaker is not connected\n\r");
        return;
    }

    if (g_audio.deviceIsUsed == 0)
    {
        usb_echo("  err: Audio Speaker is not Ready\n\r");
        return;
    }
    if (channel == 1U)
    {
        /* Send set mute request */
        if ((g_hostCurVolume + HOST_VOLUME_STEP) > HOST_MAX_VOLUME)
        {
            g_hostCurVolume = HOST_MAX_VOLUME;
            i               = 0U;
        }
        else
        {
            g_hostCurVolume += HOST_VOLUME_STEP;
            i = 1U;
        }
        g_curVolume += (int16_t)(i * HOST_VOLUME_STEP * g_deviceVolumeStep);
        g_curVol[0] = (int8_t)((uint16_t)(g_curVolume)&0x00FF);
        g_curVol[1] = (int8_t)((uint16_t)(g_curVolume) >> 8U);
    }

    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
}

/*!
 * @brief host usb audio decrease volume request.
 *
 * This function implements the host audio decrease volume request action, it is used to decrease the volume of audio
 * device.
 *
 * @param channel   the audio device channel number.
 */
void Audio_DecreaseVolumeRequest(uint8_t channel)
{
    static uint32_t i;
    uint8_t max_audio_channel;
    max_audio_channel = g_pFormatTypeDesc->bnrchannels;
    if (channel > max_audio_channel)
    {
        usb_echo("  err: Channel number larger than max channel\n\r");
        return;
    }
    if (g_audio.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Speaker is not connected\n\r");
        return;
    }

    if (g_audio.deviceIsUsed == 0U)
    {
        usb_echo("  err: Audio Speaker is not Ready\n\r");
        return;
    }
    if (channel == 1U)
    {
        if (g_hostCurVolume < (HOST_VOLUME_STEP + HOST_MIN_VOLUME))
        {
            g_hostCurVolume = HOST_MIN_VOLUME;
            i               = 0U;
        }
        else
        {
            g_hostCurVolume -= HOST_VOLUME_STEP;
            i = 1U;
        }
        g_curVolume -= (int16_t)(i * HOST_VOLUME_STEP * g_deviceVolumeStep);
        g_curVol[0] = (int8_t)((uint16_t)(g_curVolume)&0x00FF);
        g_curVol[1] = (int8_t)((uint16_t)(g_curVolume) >> 8U);
    }

    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
}

/*!
 * @brief host audio callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle           device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode              callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
usb_status_t USB_HostAudioEvent(usb_device_handle deviceHandle,
                                usb_host_configuration_handle configurationHandle,
                                uint32_t eventCode)
{
    uint8_t id;
    usb_host_configuration_t *configuration_ptr;
    uint8_t interface_g_index;
    usb_host_interface_t *interface_ptr;
    uint32_t info_value;
    usb_status_t status = kStatus_USB_Success;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration_ptr         = (usb_host_configuration_t *)configurationHandle;
            g_audioDeviceHandle       = NULL;
            g_audioOutControlifHandle = NULL;
            g_audioOutStreamifHandle  = NULL;
            for (interface_g_index = 0U; interface_g_index < configuration_ptr->interfaceCount; ++interface_g_index)
            {
                interface_ptr = &configuration_ptr->interfaceList[interface_g_index];
                id            = interface_ptr->interfaceDesc->bInterfaceClass;
                if (id != USB_AUDIO_CLASS_CODE)
                {
                    continue;
                }
                id = interface_ptr->interfaceDesc->bInterfaceSubClass;
                if (id == USB_AUDIO_SUBCLASS_CODE_CONTROL)
                {
                    g_audioDeviceHandle       = deviceHandle;
                    g_audioOutControlifHandle = interface_ptr;
                    continue;
                }
                else if (id == USB_AUDIO_SUBCLASS_CODE_AUDIOSTREAMING)
                {
                    g_audioDeviceHandle      = deviceHandle;
                    g_audioOutStreamifHandle = interface_ptr;
                    continue;
                }
                else
                {
                    continue;
                }
            }
            if (g_audioDeviceHandle != NULL)
            {
                return kStatus_USB_Success;
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_audio.devState == kStatus_DEV_Idle)
            {
                if ((g_audioDeviceHandle != NULL) && (g_audioOutControlifHandle != NULL))
                {
                    g_audio.devState          = kStatus_DEV_Attached;
                    g_audio.deviceHandle      = g_audioDeviceHandle;
                    g_audio.controlIntfHandle = g_audioOutControlifHandle;
                    g_audio.streamIntfHandle  = g_audioOutStreamifHandle;

                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &info_value);
                    usb_echo("hid audio attached:pid=0x%x", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &info_value);
                    usb_echo("vid=0x%x ", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &info_value);
                    usb_echo("address=%d\r\n", info_value);
                }
            }
            else
            {
                usb_echo("not idle audio instance\r\n");
            }
            break;

        case kUSB_HostEventDetach:
            if (g_audio.devState != kStatus_DEV_Idle)
            {
                g_audio.devState = kStatus_DEV_Detached;
            }
            break;

        default:
            break;
    }
    return status;
}
