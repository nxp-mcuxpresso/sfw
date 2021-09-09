/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_usart.h"
#include "fsl_common.h"
#include "board.h"
#include "usb_serial_port.h"

extern void USART_HandleIRQ(USART_Type *base, usart_handle_t *handle);

typedef struct _usart_status_struct
{
    usart_handle_t usartHandle;
    uint8_t isUsed;
} usart_status_struct_t;

usart_status_struct_t usartStatus[USB_SERIAL_PORT_INSTANCE_COUNT];

/*!
 * @brief usart callback function.
 *
 *This callback will be called if the usart has get specific num(USB_HOST_CDC_USART_RX_MAX_LEN) char.
 *
 * @param instance           instancehandle.
 * @param usartState           callback event code, please reference to enumeration host_event_t.
 *
 */
void USART_SeralPortTransferCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    usb_serial_port_handle_t *serialPortHandle = (usb_serial_port_handle_t *)userData;
    if (NULL != serialPortHandle->callback.callbackFunction)
    {
        serialPortHandle->callback.callbackFunction(handle, (((uint32_t)status) % 100) + 1,
                                                    serialPortHandle->callback.callbackParam);
    }
}

usb_serial_port_status_t USB_SerialPortInit(uint8_t instance,
                                            const usb_serial_port_config_t *config,
                                            usb_serial_port_callback_struct_t *callback,
                                            uint32_t sourceClockHz,
                                            usb_serial_port_handle_t *handle)
{
    USART_Type *usart[] = USART_BASE_PTRS;
    int i;
    usart_config_t usartConfiguration;

    if (instance >= (sizeof(usart) / sizeof(USART_Type *)))
    {
        return kStatus_USB_SERIAL_PORT_InvalidParameter;
    }

    if ((NULL == handle) || (NULL == callback))
    {
        return kStatus_USB_SERIAL_PORT_InvalidParameter;
    }

    handle->serialPortHandle = NULL;

    for (i = 0; i < USB_SERIAL_PORT_INSTANCE_COUNT; i++)
    {
        if (0 == usartStatus[i].isUsed)
        {
            handle->serialPortHandle = &usartStatus[i];
            usartStatus[i].isUsed    = 1U;
            break;
        }
    }
    if (NULL == handle->serialPortHandle)
    {
        return kStatus_USB_SERIAL_PORT_Busy;
    }

    handle->callback.callbackFunction = callback->callbackFunction;
    handle->callback.callbackParam    = callback->callbackParam;

    USART_GetDefaultConfig(&usartConfiguration);

    usartConfiguration.baudRate_Bps = config->baudRate_Bps;
    usartConfiguration.enableTx     = config->enableTx;
    usartConfiguration.enableRx     = config->enableRx;
    USART_Init(usart[instance], &usartConfiguration, sourceClockHz);

    handle->baseReg  = usart[instance];
    handle->instance = instance;

    USART_TransferCreateHandle(usart[instance],
                               (usart_handle_t *)&((usart_status_struct_t *)handle->serialPortHandle)->usartHandle,
                               USART_SeralPortTransferCallback, handle);

    return kStatus_USB_SERIAL_PORT_Success;
}

usb_serial_port_status_t USB_SerialPortRecv(usb_serial_port_handle_t *handle,
                                            usb_serial_port_xfer_t *xfer,
                                            size_t *receivedBytes)
{
    return (usb_serial_port_status_t)(
        USART_TransferReceiveNonBlocking(
            handle->baseReg, (usart_handle_t *)&((usart_status_struct_t *)handle->serialPortHandle)->usartHandle,
            (usart_transfer_t *)xfer, receivedBytes) %
            100 +
        1);
}

usb_serial_port_status_t USB_SerialPortSend(usb_serial_port_handle_t *handle, usb_serial_port_xfer_t *xfer)
{
    return (usb_serial_port_status_t)(
        USART_TransferSendNonBlocking(
            handle->baseReg, (usart_handle_t *)&((usart_status_struct_t *)handle->serialPortHandle)->usartHandle,
            (usart_transfer_t *)xfer) %
            100 +
        1);
}

usb_serial_port_status_t USB_SerialPortDeinit(usb_serial_port_handle_t *handle)
{
    USART_Deinit(handle->baseReg);
    return kStatus_USB_SERIAL_PORT_Success;
}

void USB_SerialPortIRQHandler(usb_serial_port_handle_t *handle)
{
    USART_TransferHandleIRQ(handle->baseReg,
                            (usart_handle_t *)&((usart_status_struct_t *)handle->serialPortHandle)->usartHandle);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}
