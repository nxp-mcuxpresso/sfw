/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "string.h"
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "pd_i2c_over_vdm.h"
#include "pd_command_interface.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "board.h"
#include "pd_power_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status = kStatus_PD_Error;

    switch (event)
    {
        /* hard reset */
        case PD_DPM_SNK_HARD_RESET_REQUEST:
        case PD_DPM_SRC_HARD_RESET_REQUEST:
            status = PD_DpmHardResetCallback(callbackParam);
            break;

        /* rdo request and power negotiation */
        case PD_DPM_SRC_RDO_REQUEST:
            status = PD_DpmSrcRDORequestCallback(callbackParam, ((pd_negotiate_power_request_t *)param)->rdo,
                                                 &(((pd_negotiate_power_request_t *)param)->negotiateResult));
            break;
        case PD_DPM_SRC_CONTRACT_STILL_VALID:
            status = PD_DpmSrcPreContractStillValidCallback(callbackParam, (uint8_t *)param);
            break;
        case PD_DPM_SRC_SEND_SRC_CAP_FAIL:
            status = PD_DpmSrcRDOResultCallback(callbackParam, 0, kCommandResult_Error);
            break;
        case PD_DPM_SRC_RDO_SUCCESS:
            status = PD_DpmSrcRDOResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SRC_RDO_FAIL:
            status = PD_DpmSrcRDOResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

        /* goto min */
        case PD_DPM_SRC_GOTOMIN_FAIL:
            status = PD_DpmSrcGotoMinResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;
        case PD_DPM_SRC_GOTOMIN_SUCCESS:
            status = PD_DpmSrcGotoMinResultCallback(callbackParam, 1, 0);
            break;

        /* get sink cap */
        case PD_DPM_GET_PARTNER_SNK_CAP_SUCCESS:
            status = PD_DpmReceivePartnerSnkCapsCallback(callbackParam, (pd_capabilities_t *)param);
            break;
        case PD_DPM_GET_PARTNER_SNK_CAP_FAIL:
            status = PD_DpmGetPartnerSnkCapsFailCallback(callbackParam, *((uint8_t *)param));
            break;

        /* soft reset */
        case PD_DPM_SOFT_RESET_SUCCESS:
        case PD_DPM_SOFT_RESET_REQUEST:
            status = PD_DpmSoftResetCallback(callbackParam);
            break;
        case PD_DPM_SOFT_RESET_FAIL:
            /* in normal situation, app don't need process it PD stack will do hard_reset if soft_reset fail. */
            break;

/* unstructured vdm */
#if 0
        case PD_DPM_UNSTRUCTURED_VDM_RECEIVED:
            status = PD_DpmUnstructuredVDMReceivedCallback(callbackParam, (pd_unstructured_vdm_command_param_t *)param);
            break;
        case PD_DPM_SEND_UNSTRUCTURED_VDM_SUCCESS:
            status = PD_DpmUnstructuredVDMSendResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SEND_UNSTRUCTURED_VDM_FAIL:
            status = PD_DpmUnstructuredVDMSendResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;
#endif
        case PD_DPM_UNSTRUCTURED_VDM_RECEIVED:
        case PD_DPM_SEND_UNSTRUCTURED_VDM_SUCCESS:
        case PD_DPM_SEND_UNSTRUCTURED_VDM_FAIL:
            if (((pd_app_t *)callbackParam)->uVdmCallback != NULL)
            {
                ((pd_app_t *)callbackParam)->uVdmCallback(((pd_app_t *)callbackParam)->uVdmCallbackParam, event, param);
            }
            break;

        default:
            break;
    }
    return status;
}

/* register unstructured VDM callback
 * return value: 0 - success, other values - fail */
pd_status_t PD_UnstructuredVDMSetCallback(pd_handle pdHandle,
                                          pd_unstructured_vdm_callback_t callbackFn,
                                          void *callbackParam)
{
    pd_app_t *pdAppInstance = NULL;
    uint8_t index;

    /* get the pd app instance */
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index]->pdHandle == pdHandle)
        {
            pdAppInstance = g_PDAppInstanceArray[index];
            break;
        }
    }

    if (pdAppInstance == NULL)
    {
        return kStatus_PD_Error;
    }
    pdAppInstance->uVdmCallback      = callbackFn;
    pdAppInstance->uVdmCallbackParam = callbackParam;

    return kStatus_PD_Success;
}
