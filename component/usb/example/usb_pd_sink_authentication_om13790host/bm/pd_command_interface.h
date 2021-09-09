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
pd_status_t PD_DpmPowerRoleSwapRequestCallback(void *callbackParam, uint8_t frSwap, uint8_t *evaluateResult);
pd_status_t PD_DpmPowerRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);
pd_status_t PD_DpmDataRoleSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult);
pd_status_t PD_DpmDataRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);
pd_status_t PD_DpmVconnSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult);
pd_status_t PD_DpmVconnSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);
pd_status_t PD_DpmReceivePartnerSrcCapsCallback(void *callbackParam, pd_capabilities_t *caps);
pd_status_t PD_DpmGetPartnerSrcCapsFailCallback(void *callbackParam, uint8_t failResultType);
pd_status_t PD_DpmSnkGetRequestRDOCallback(void *callbackParam, pd_rdo_t *rdo);
pd_status_t PD_DpmSnkRDOResultCallback(void *callbackParam, uint8_t success, uint32_t failResultTypeOrRDO);
pd_status_t PD_DpmSnkGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);
pd_status_t PD_DpmSVDMRequestCallback(void *callbackParam, pd_svdm_command_request_t *svdmRequest);
/* pd_status_t PD_DpmSVDMResultCallback(void *callbackParam, uint8_t success, pd_svdm_command_result_t *svdmResult); */
pd_status_t PD_DpmUnstructuredVDMReceivedCallback(void *callbackParam,
                                                  pd_unstructured_vdm_command_param_t *unstructuredVDMParam);
pd_status_t PD_DpmUnstructuredVDMSendResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType);

/*!
 * @brief Reset the cable related functions.
 *
 * @param callbackParam PD_InstanceInit function pass the parameter to PD stack.
 * @param receiver      if Ture: the port is cable reset self functions.
 *                      if False: the port is DFP reset cable related functions.
 *
 * @retval kStatus_PD_Success  process the callback success
 * @retval kStatus_PD_Error    error
 */
/* pd_status_t PD_DpmCableResetRquestCallback(void *callbackParam, uint8_t receiver); */

/* register unstructured VDM callback */
pd_status_t PD_UnstructuredVDMSetCallback(pd_handle pdHandle,
                                          pd_unstructured_vdm_callback_t callbackFn,
                                          void *callbackParam);

#endif
