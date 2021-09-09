/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_COMMAND_INTERFACE_H__
#define __PD_COMMAND_INTERFACE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _pd_dpm_info_type
{
    kInfoType_SrcExtCap,
    kInfoType_Status,
    kInfoType_BatteryCap,
    kInfoType_BatteryStatus,
    kInfoType_ManufacturerInfo,
} pd_dpm_info_type_t;

/* unstructured VDM callback
 */
typedef pd_status_t (*pd_unstructured_vdm_callback_t)(void *callbackParam, uint32_t event, void *param);

/*******************************************************************************
 * API
 ******************************************************************************/

pd_status_t PD_DpmHardResetCallback(void *callbackParam);
pd_status_t PD_DpmSoftResetCallback(void *callbackParam);
pd_status_t PD_DpmReceivePartnerSnkCapsCallback(void *callbackParam, pd_capabilities_t *caps);
pd_status_t PD_DpmGetPartnerSnkCapsFailCallback(void *callbackParam, uint8_t failResultType);
pd_status_t PD_DpmSrcRDORequestCallback(void *callbackParam, pd_rdo_t rdo, uint8_t *negotiateResult);
pd_status_t PD_DpmSrcPreContractStillValidCallback(void *callbackParam, uint8_t *isStillValid);
pd_status_t PD_DpmSrcRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);
pd_status_t PD_DpmSrcGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);
pd_status_t PD_DpmUnstructuredVDMReceivedCallback(void *callbackParam,
                                                  pd_unstructured_vdm_command_param_t *unstructuredVDMParam);
pd_status_t PD_DpmUnstructuredVDMSendResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);

/* register unstructured VDM callback */
pd_status_t PD_UnstructuredVDMSetCallback(pd_handle pdHandle,
                                          pd_unstructured_vdm_callback_t callbackFn,
                                          void *callbackParam);

#endif
