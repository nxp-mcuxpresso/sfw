/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_cdc_rndis.h"
#include "usb_host_cdc.h"
#include "host_cdc_rndis.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t send_message[MESSAGE_LEN];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t get_message[MESSAGE_LEN];

usb_host_rndis_instance_struct_t g_RndisInstance = {0};
uint8_t responseAvailable;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief host cdc interrupt transfer callback.
 *
 * This function is used as callback function for interrupt transfer . Interrupt transfer is used to implement
 * asynchronous notification of UART status as pstn sepc. This callback suppose the device will return SerialState
 * notification. If there is need to suppose other notification ,please refer pstn spec 6.5 and cdc spec6.3.
 * @param param    the host cdc instance pointer.
 * @param data     data buffer pointer.
 * @param dataLength data length.
 * @status         transfer result status.
 */
void USB_HostCdcRndisInterruptCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_cdc_acm_state_struct_t *state = (usb_host_cdc_acm_state_struct_t *)data;

    if (status != kStatus_USB_Success)
    {
        if (status == kStatus_USB_TransferCancel)
        {
            usb_echo("cdc transfer cancel\r\n");
        }
        else
        {
            usb_echo("cdc control transfer error\r\n");
        }
    }
    else
    { /*more information about SerialState ,please pstn spec 6.5.4 */
        usb_echo("get serial state value = %d\r\n", state->bmstate);
        responseAvailable = 1;
    }
}
/*!
 * @brief host cdc rndis control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param    the host cdc rndis instance pointer.
 * @param data     data buffer pointer.
 * @param dataLength data length.
 * @status         transfer result status.
 */
void USB_HostCdcRndisControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_rndis_instance_struct_t *rndisInstance = (usb_host_rndis_instance_struct_t *)param;

    if (status != kStatus_USB_Success)
    {
        usb_echo("data transfer error = %d , status \r\n");
        return;
    }

    if (rndisInstance->runWaitState == kUSB_HostCdcRndisRunWaitSetControlInterface)
    {
        rndisInstance->runState = kUSB_HostCdcRndisRunSetControlInterfaceDone;
    }
    else if (rndisInstance->runWaitState == kUSB_HostCdcRndisRunWaitSetDataInterface)
    {
        rndisInstance->runState = kUSB_HostCdcRndisRunSetDataInterfaceDone;
    }
    else if (rndisInstance->runWaitState == kUSB_HostCdcRndisRunWaitInitMsg)
    {
        rndisInstance->runState = kUSB_HostCdcRndisRunWaitInitMsgDone;
    }
    else if (rndisInstance->runWaitState == kUSB_HostCdcRndisRunWaitMaxmumFrame)
    {
        rndisInstance->runState = kUSB_HostCdcRndisRunWaitMaxmumFrameDone;
    }
    else if (rndisInstance->runWaitState == kUSB_HostCdcRndisRunWaitSetMsg)
    {
        rndisInstance->runState = kUSB_HostCdcRndisRunWaitSetMsgDone;
    }
    else if (rndisInstance->runWaitState == kUSB_HostCdcRndisRunWaitGetEncapsulatedCommand)
    {
        rndisInstance->runState = kUSB_HostCdcRndisRunGetEncapsulatedResponseDone;
        if (rndisInstance->previousRunState == kUSB_HostCdcRndisRunWaitInitMsgDone)
        {
            rndisInstance->runState = kUSB_HostCdcRndisRunWaitMaxmumFrame;
        }
        else if (rndisInstance->previousRunState == kUSB_HostCdcRndisRunWaitMaxmumFrameDone)
        {
            rndisInstance->runState = kUSB_HostCdcRndisRunWaitSetMsg;
        }
        else if (rndisInstance->previousRunState == kUSB_HostCdcRndisRunWaitSetMsgDone)
        {
            rndisInstance->runState = kUSB_HostCdcRndisRunGetState;
        }
    }
    else
    {
    }
}

/*!
 * @brief host rndis task function.
 *
 * This function implements the host cdc action, it is used to create task.
 *
 * @param param   the host rndis instance pointer.
 */
void USB_HosCdcRndisTask(void *param)
{
    usb_status_t status                             = kStatus_USB_Success;
    usb_host_rndis_instance_struct_t *rndisInstance = (usb_host_rndis_instance_struct_t *)param;
    uint32_t filtertype;
    /* device state changes */
    if (rndisInstance->deviceState != rndisInstance->previousState)
    {
        rndisInstance->previousState = rndisInstance->deviceState;
        switch (rndisInstance->deviceState)
        {
            case kStatus_DEV_Idle:
                break;
            case kStatus_DEV_Attached:
                rndisInstance->runState = kUSB_HostCdcRndisRunSetControlInterface;
                status                  = USB_HostCdcInit(rndisInstance->deviceHandle, &rndisInstance->classHandle);
                usb_echo("rndis device attached\r\n");
                break;
            case kStatus_DEV_Detached:
                rndisInstance->deviceState = kStatus_DEV_Idle;
                rndisInstance->runState    = kUSB_HostCdcRndisRunIdle;
                USB_HostCdcDeinit(rndisInstance->deviceHandle, rndisInstance->classHandle);
                rndisInstance->dataInterfaceHandle    = NULL;
                rndisInstance->classHandle            = NULL;
                rndisInstance->controlInterfaceHandle = NULL;
                rndisInstance->deviceHandle           = NULL;
                usb_echo("rndis device detached\r\n");
                break;
            default:
                break;
        }
    }

    /* run state */
    switch (rndisInstance->runState)
    {
        case kUSB_HostCdcRndisRunIdle:
            break;
        case kUSB_HostCdcRndisRunSetControlInterface:
            rndisInstance->runWaitState = kUSB_HostCdcRndisRunWaitSetControlInterface;
            rndisInstance->runState     = kUSB_HostCdcRndisRunIdle;
            if (USB_HostCdcSetControlInterface(rndisInstance->classHandle, rndisInstance->controlInterfaceHandle, 0,
                                               USB_HostCdcRndisControlCallback, rndisInstance) != kStatus_USB_Success)
            {
                usb_echo("set control interface error\r\n");
            }
            break;
        case kUSB_HostCdcRndisRunSetControlInterfaceDone:
            rndisInstance->runWaitState = kUSB_HostCdcRndisRunWaitSetDataInterface;
            rndisInstance->runState     = kUSB_HostCdcRndisRunIdle;
            if (USB_HostCdcSetDataInterface(rndisInstance->classHandle, rndisInstance->dataInterfaceHandle, 0,
                                            USB_HostCdcRndisControlCallback, rndisInstance) != kStatus_USB_Success)
            {
                usb_echo("set data interface error\r\n");
            }
            break;
        case kUSB_HostCdcRndisRunSetDataInterfaceDone:
            rndisInstance->runWaitState = kUSB_HostCdcRndisRunWaitInitMsg;
            rndisInstance->runState     = kUSB_HostCdcRndisRunIdle;
            if (USB_HostRndisInitMsg(rndisInstance->classHandle, &send_message[0], MESSAGE_LEN,
                                     USB_HostCdcRndisControlCallback, rndisInstance))
            {
                usb_echo("Error in Init message\r\n");
            }
            break;
        case kUSB_HostCdcRndisRunWaitInitMsgDone:
            if (responseAvailable == 0)
            {
                if (USB_HostCdcInterruptRecv(rndisInstance->classHandle, (uint8_t *)&rndisInstance->state,
                                             sizeof(rndisInstance->state), USB_HostCdcRndisInterruptCallback,
                                             rndisInstance) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostCdcInterruptRecv: %x\r\n", status);
                }
            }
            else if (responseAvailable)
            {
                responseAvailable = 0;
                /*remember the previous set encapsulated command*/
                rndisInstance->previousRunState = kUSB_HostCdcRndisRunWaitInitMsgDone;
                rndisInstance->runWaitState     = kUSB_HostCdcRndisRunWaitGetEncapsulatedCommand;
                rndisInstance->runState         = kUSB_HostCdcRndisRunIdle;

                if (USB_HostCdcGetEncapsulatedResponse(rndisInstance->classHandle, &get_message[0], MESSAGE_LEN,
                                                       USB_HostCdcRndisControlCallback, rndisInstance))
                {
                    usb_echo("Error in Init message\r\n");
                }
            }
            break;
        case kUSB_HostCdcRndisRunWaitMaxmumFrame:
            rndisInstance->previousRunState = kUSB_HostCdcRndisRunWaitMaxmumFrame;
            rndisInstance->runWaitState     = kUSB_HostCdcRndisRunWaitMaxmumFrame;
            rndisInstance->runState         = kUSB_HostCdcRndisRunIdle;

            if (USB_HostRndisQueryMsg(rndisInstance->classHandle, OID_GEN_MAXIMUM_FRAME_SIZE, &send_message[0],
                                      MESSAGE_LEN, 0, 0, NULL, USB_HostCdcRndisControlCallback, rndisInstance))
            {
                usb_echo("Error in Init message\r\n");
            }
            break;

        case kUSB_HostCdcRndisRunWaitMaxmumFrameDone:
            if (responseAvailable == 0)
            {
                if (USB_HostCdcInterruptRecv(rndisInstance->classHandle, (uint8_t *)&rndisInstance->state,
                                             sizeof(rndisInstance->state), USB_HostCdcRndisInterruptCallback,
                                             rndisInstance) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostCdcInterruptRecv: %x\r\n", status);
                }
            }
            else if (responseAvailable)
            {
                responseAvailable               = 0;
                rndisInstance->previousRunState = kUSB_HostCdcRndisRunWaitMaxmumFrameDone;
                rndisInstance->runWaitState     = kUSB_HostCdcRndisRunWaitGetEncapsulatedCommand;
                rndisInstance->runState         = kUSB_HostCdcRndisRunIdle;

                if (USB_HostCdcGetEncapsulatedResponse(rndisInstance->classHandle, &get_message[0], MESSAGE_LEN,
                                                       USB_HostCdcRndisControlCallback, rndisInstance))
                {
                    usb_echo("Error in Init message\r\n");
                }
            }
            break;
        case kUSB_HostCdcRndisRunWaitSetMsg:
            rndisInstance->previousRunState = kUSB_HostCdcRndisRunWaitSetMsg;
            rndisInstance->runWaitState     = kUSB_HostCdcRndisRunWaitSetMsg;
            rndisInstance->runState         = kUSB_HostCdcRndisRunIdle;
            filtertype                      = NDIS_PACKET_TYPE_DIRECTED;
            if (USB_HostRndisSetMsg(rndisInstance->classHandle, OID_GEN_CURRENT_PACKET_FILTER, &send_message[0],
                                    MESSAGE_LEN, 20U, (sizeof(filtertype)), &filtertype,
                                    USB_HostCdcRndisControlCallback, rndisInstance))
            {
                usb_echo("Error in Init message\r\n");
            }
            break;
        case kUSB_HostCdcRndisRunWaitSetMsgDone:
            if (responseAvailable == 0)
            {
                if (USB_HostCdcInterruptRecv(rndisInstance->classHandle, (uint8_t *)&rndisInstance->state,
                                             sizeof(rndisInstance->state), USB_HostCdcRndisInterruptCallback,
                                             rndisInstance) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostCdcInterruptRecv: %x\r\n", status);
                }
            }
            else if (responseAvailable)
            {
                responseAvailable               = 0;
                rndisInstance->previousRunState = kUSB_HostCdcRndisRunWaitSetMsgDone;
                rndisInstance->runWaitState     = kUSB_HostCdcRndisRunWaitGetEncapsulatedCommand;
                rndisInstance->runState         = kUSB_HostCdcRndisRunIdle;

                if (USB_HostCdcGetEncapsulatedResponse(rndisInstance->classHandle, &get_message[0], MESSAGE_LEN,
                                                       USB_HostCdcRndisControlCallback, rndisInstance))
                {
                    usb_echo("Error in Init message\r\n");
                }
            }
            break;
        case kUSB_HostCdcRndisRunGetState:
            rndisInstance->runState = kUSB_HostCdcRndisRunGetStateDone;
            rndisInstance->runState = kUSB_HostCdcRndisRunIdle;
            if (USB_HostCdcInterruptRecv(rndisInstance->classHandle, (uint8_t *)&rndisInstance->state,
                                         sizeof(rndisInstance->state), USB_HostCdcRndisInterruptCallback,
                                         rndisInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostCdcInterruptRecv: %x\r\n", status);
            }
            break;
            //        case kUSB_HostCdcRndisRunGetStateDone:
            //            rndisInstance->runState = kUSB_HostCdcRndisRunIdle;
            break;
        default:
            break;
    }
}

usb_status_t USB_HostCdcRndisEvent(usb_device_handle deviceHandle,
                                   usb_host_configuration_handle configurationHandle,
                                   uint32_t event_code)
{
    usb_status_t status;
    uint8_t id;
    usb_host_configuration_t *configuration;
    uint8_t interface_index;
    usb_host_interface_t *hostInterface;
    uint32_t info_value;

    status = kStatus_USB_Success;

    switch (event_code)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;

            for (interface_index = 0; interface_index < configuration->interfaceCount; ++interface_index)
            {
                hostInterface = &configuration->interfaceList[interface_index];
                id            = hostInterface->interfaceDesc->bInterfaceClass;

                if (id != USB_HOST_CDC_COMMUNICATIONS_CLASS_CODE)
                {
                    continue;
                }
                id = hostInterface->interfaceDesc->bInterfaceSubClass;
                if (id != USB_HOST_CDC_SUBCLASS_ACM_CODE)
                {
                    continue;
                }
                else
                {
                    /* the interface is supported by the application */
                    g_RndisInstance.controlInterfaceHandle = hostInterface;
                }
            }
            for (interface_index = 0; interface_index < configuration->interfaceCount; ++interface_index)
            {
                hostInterface = &configuration->interfaceList[interface_index];
                id            = hostInterface->interfaceDesc->bInterfaceClass;

                if (id != USB_HOST_CDC_DATA_CLASS_CODE)
                {
                    continue;
                }
                id = hostInterface->interfaceDesc->bInterfaceSubClass;
                if (id != USB_HOST_CDC_DATA_SUBCLASS_CODE)
                {
                    continue;
                }
                id = hostInterface->interfaceDesc->bInterfaceProtocol;
                if (id != USB_HOST_CDC_DATA_PROTOCOL_CODE)
                {
                    continue;
                }
                else
                {
                    g_RndisInstance.dataInterfaceHandle = hostInterface;
                }
            }
            g_RndisInstance.deviceHandle = deviceHandle;
            if ((NULL != g_RndisInstance.dataInterfaceHandle) && (NULL != g_RndisInstance.controlInterfaceHandle))
            {
                status = kStatus_USB_Success;
            }
            else
            {
                status = kStatus_USB_NotSupported;
            }
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_RndisInstance.deviceState == kStatus_DEV_Idle)
            {
                if ((g_RndisInstance.deviceHandle != NULL) && (g_RndisInstance.dataInterfaceHandle != NULL) &&
                    (g_RndisInstance.controlInterfaceHandle != NULL))
                {
                    g_RndisInstance.deviceState = kStatus_DEV_Attached;

                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &info_value);
                    usb_echo("device cdc attached:\r\npid=0x%x", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &info_value);
                    usb_echo("vid=0x%x ", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &info_value);
                    usb_echo("address=%d\r\n", info_value);
                }
            }
            else
            {
                usb_echo("not idle rndis instance\r\n");
            }
            break;

        case kUSB_HostEventDetach:
            if (g_RndisInstance.deviceState != kStatus_DEV_Idle)
            {
                g_RndisInstance.deviceState = kStatus_DEV_Detached;
            }
            break;

        default:
            break;
    }
    return status;
}
