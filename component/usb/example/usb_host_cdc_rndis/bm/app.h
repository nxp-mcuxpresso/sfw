/*
 * Copyright 2018 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_H__
#define __APP_H__
#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if ((defined USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI))
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif /* USB_HOST_CONFIG_KHCI */
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif /* USB_HOST_CONFIG_EHCI */
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI))
#define CONTROLLER_ID kUSB_ControllerOhci0
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS))
#define CONTROLLER_ID kUSB_ControllerIp3516Hs0
#endif /* USB_HOST_CONFIG_IP3516HS */

#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif

/*! @brief host app device attach/detach status */
typedef enum HostAppState
{
    kStatus_DEV_Idle = 0, /*!< there is no device attach/detach */
    kStatus_DEV_Attached, /*!< device is attached */
    kStatus_DEV_Detached, /*!< device is detached */
} host_app_state;
/*!
 * @brief host cdc task function.
 *
 * This function implements the host mouse action, it is used to create task.
 *
 * @param param   the host cdc instance pointer.
 */
extern void USB_HosCdcRndisTask(void *param);

/*!
 * @brief host cdc callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle           device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param event_code           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
extern usb_status_t USB_HostCdcRndisEvent(usb_device_handle deviceHandle,
                                          usb_host_configuration_handle configurationHandle,
                                          uint32_t event_code);
#endif /* __APP_H__ */
