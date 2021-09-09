/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DFU_H__
#define __USB_DEVICE_DFU_H__

/*!
 * @addtogroup usb_device_dfu_drv
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief The class code of the DFU class */
#define USB_DEVICE_CONFIG_DFU_CLASS_CODE (0xFEU)

/*! @brief DFU class request */
#define USB_DEVICE_DFU_DETACH (0x00U)
#define USB_DEVICE_DFU_DNLOAD (0x01U)
#define USB_DEVICE_DFU_UPLOAD (0x02U)
#define USB_DEVICE_DFU_GETSTATUS (0x03U)
#define USB_DEVICE_DFU_CLRSTATUS (0x04U)
#define USB_DEVICE_DFU_GETSTATE (0x05U)
#define USB_DEVICE_DFU_ABORT (0x06U)

/*! @brief Available common EVENT types in dfu class callback */
typedef enum _usb_device_dfu_event
{
    kUSB_DeviceDfuEventDetach = 0x01U, /*!< Detach request */
    kUSB_DeviceDfuEventDownLoad,       /*!< Download request */
    kUSB_DeviceDfuEventUpLoad,         /*!< Upload request */
    kUSB_DeviceDfuEventGetStatus,      /*!< Get status request */
    kUSB_DeviceDfuEventClearStatus,    /*!< Clear status request */
    kUSB_DeviceDfuEventGetState,       /*!< Get state request */
    kUSB_DeviceDfuEventAbort,          /*!< Abort request */
} usb_device_dfu_event_t;

/*! @}*/

#endif /* __USB_DEVICE_DFU_H__ */
