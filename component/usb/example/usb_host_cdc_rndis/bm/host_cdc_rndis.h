/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HOST_CDC_RNDIS_H__
#define __HOST_CDC_RNDSI_H__

typedef struct _usb_host_rndis_instance_struct
{
    usb_device_handle deviceHandle;
    usb_host_class_handle classHandle;
    usb_host_interface_handle controlInterfaceHandle;
    usb_host_interface_handle dataInterfaceHandle;
    usb_host_cdc_acm_state_struct_t state;
    uint8_t deviceState;
    uint8_t previousState;
    uint8_t runState;
    uint8_t previousRunState;
    uint8_t runWaitState;
} usb_host_rndis_instance_struct_t;

/*! @brief host app run status */
typedef enum HostCdcRndisRunState
{
    kUSB_HostCdcRndisRunIdle = 0,                    /*!< idle */
    kUSB_HostCdcRndisRunSetControlInterface,         /*!< execute set interface code */
    kUSB_HostCdcRndisRunWaitSetControlInterface,     /*!< wait set interface done */
    kUSB_HostCdcRndisRunSetControlInterfaceDone,     /*!< set interface is done, execute next step */
    kUSB_HostCdcRndisRunSetDataInterface,            /*!< execute set interface code */
    kUSB_HostCdcRndisRunWaitSetDataInterface,        /*!< wait set interface done */
    kUSB_HostCdcRndisRunSetDataInterfaceDone,        /*!< set interface is done, execute next step */
    kUSB_HostCdcRndisRunWaitInitMsg,                 /*!< wait init message done */
    kUSB_HostCdcRndisRunWaitInitMsgDone,             /*!< init message done, execute next step */
    kUSB_HostCdcRndisRunWaitMaxmumFrame,             /*!< wait query message done */
    kUSB_HostCdcRndisRunWaitMaxmumFrameDone,         /*!< init query done, execute next step */
    kUSB_HostCdcRndisRunWaitSetMsg,                  /*!< wait set message done */
    kUSB_HostCdcRndisRunWaitSetMsgDone,              /*!< init query done, execute next step */
    kUSB_HostCdcRndisRunWaitDataSend,                /*!< wait data send done */
    kUSB_HostCdcRndisRunDataSendDone,                /*!< data send is done, execute next step  */
    kUSB_HostCdcRndisRunWaitDataReceived,            /*!< wait data receive done */
    kUSB_HostCdcRndisRunDataReceivedDone,            /*!< data receive is done, execute next step  */
    kUSB_HostCdcRndisRunWaitGetEncapsulatedCommand,  /*!< wait data receive done */
    kUSB_HostCdcRndisRunGetEncapsulatedResponseDone, /*!< data receive is done, execute next step  */
    kUSB_HostCdcRndisRunGetState,
    kUSB_HostCdcRndisRunGetStateDone

} host_cdc_rndis_run_state;

#endif /*__HOST_CDC_RNDIS_H__*/
