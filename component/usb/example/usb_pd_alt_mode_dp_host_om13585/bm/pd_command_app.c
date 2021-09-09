/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "usb_pd_phy.h"
#include "pd_app.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "pd_command_interface.h"
#include "pd_board_config.h"

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

static pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage)
{
    uint32_t index;
    pd_source_pdo_t sourcePDO;
    uint8_t findSourceCap = 0;

    if (pdAppInstance->partnerSourceCapNumber == 0)
    {
        return kStatus_PD_Error;
    }

    /* default rdo as 5V - 0.5A or less */
    *voltage                                = 5000;
    rdo->bitFields.objectPosition           = 1;
    rdo->bitFields.giveBack                 = 0;
    rdo->bitFields.capabilityMismatch       = 0;
    rdo->bitFields.usbCommunicationsCapable = 0;
    rdo->bitFields.noUsbSuspend             = 1;
    rdo->bitFields.operateValue             = 500 / PD_PDO_CURRENT_UNIT;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    rdo->bitFields.unchunkedSupported = 1;
#endif
    if (rdo->bitFields.operateValue > pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent)
    {
        rdo->bitFields.operateValue = pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent;
    }
    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;

    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        sourcePDO.PDOValue = pdAppInstance->partnerSourceCaps[index].PDOValue;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if ((sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT == requestVoltagemV) &&
                    (sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrentmA))
                {
                    *voltage                            = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1);
                    rdo->bitFields.operateValue         = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            case kPDO_Variable:
            {
                if ((sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltagemV) &&
                    (sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltagemV) &&
                    (sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrentmA))
                {
                    *voltage                            = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1);
                    rdo->bitFields.operateValue         = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltagemV) &&
                    (sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltagemV) &&
                    (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT >=
                     (requestVoltagemV * requestCurrentmA / 1000)))
                {
                    *voltage                      = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    rdo->bitFields.operateValue   = (requestVoltagemV * requestCurrentmA) / 1000 / PD_PDO_POWER_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            default:
                break;
        }

        if (findSourceCap)
        {
            break;
        }
    }

    if (findSourceCap)
    {
        return kStatus_PD_Success;
    }
    return kStatus_PD_Error;
}

pd_status_t PD_DpmSoftResetCallback(void *callbackParam)
{
    /* reset soft status */
    PRINTF("app soft reset\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmHardResetCallback(void *callbackParam)
{
    /* reset state, The Sink shall not draw more than iSafe0mA when VBUS is driven to vSafe0V. */
    PRINTF("hard reset request\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapRequestCallback(void *callbackParam, uint8_t frSwap, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
    uint8_t powerRole;
    pd_source_pdo_t pdo;

    if (frSwap)
    {
        *evaluateResult = kCommandResult_Accept;
    }
    else
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);

        pdo.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->deviceConfig)->sourceCaps[0];
        if ((powerRole == kPD_PowerRoleSource) && (pdo.fixedPDO.externalPowered))
        {
            if ((pdAppInstance->partnerSourceCapNumber == 0) && (pdAppInstance->partnerSinkCapNumber == 0))
            {
                *evaluateResult = kCommandResult_Wait;
                PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES, NULL);
            }
            else
            {
                if (pdAppInstance->partnerSourceCapNumber)
                {
                    if (pdAppInstance->partnerSourceCaps[0].fixedPDO.externalPowered == 0)
                    {
                        *evaluateResult = kCommandResult_Reject;
                    }
                    else
                    {
                        *evaluateResult = kCommandResult_Accept;
                    }
                }
                else
                {
                    if (pdAppInstance->partnerSinkCaps[0].fixedPDO.externalPowered == 0)
                    {
                        *evaluateResult = kCommandResult_Reject;
                    }
                    else
                    {
                        *evaluateResult = kCommandResult_Accept;
                    }
                }
            }
        }
        else
        {
            *evaluateResult = kCommandResult_Accept;
        }
    }
#else
    *evaluateResult = ((pdAppInstance->prSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
#endif
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &roleInfo);
        if (roleInfo == kPD_PowerRoleSource)
        {
            PRINTF("enter source\r\n");
        }
        else
        {
            PRINTF("enter sink\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PRINTF("power role swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("power role swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("power role swap result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    *evaluateResult = ((pdAppInstance->vconnSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &roleInfo);
        if (roleInfo == kPD_IsVconnSource)
        {
            PRINTF("vconn swap result: turn as source\r\n");
        }
        else
        {
            PRINTF("vconn swap result: not vconn source\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PRINTF("vconn swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("vconn swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("vconn swap result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDORequestCallback(void *callbackParam, pd_rdo_t rdo, uint8_t *negotiateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t accept          = 0;

    /* prepare for power supply, The power supply shall be ready to operate at the new power level within
     * tSrcReady */
    if (rdo.bitFields.objectPosition <=
        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->deviceConfig)->sourceCapCount)
    {
        pd_source_pdo_t sourcePDO;
        sourcePDO.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->deviceConfig)
                                 ->sourceCaps[rdo.bitFields.objectPosition - 1];
        accept = 1;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if (rdo.bitFields.operateValue > sourcePDO.fixedPDO.maxCurrent)
                {
                    accept = 0;
                }
                break;
            }

            case kPDO_Variable:
            {
                if (rdo.bitFields.operateValue > sourcePDO.variablePDO.maxCurrent)
                {
                    accept = 0;
                }
                break;
            }

            case kPDO_Battery:
            {
                if (rdo.bitFields.operateValue > sourcePDO.batteryPDO.maxAllowPower)
                {
                    accept = 0;
                }
                break;
            }

            default:
                break;
        }
    }

    if (accept)
    {
        pdAppInstance->sinkRequestRDO.rdoVal = rdo.rdoVal;
        *negotiateResult                     = kCommandResult_Accept;
    }
    else
    {
        *negotiateResult = kCommandResult_Reject;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcPreContractStillValidCallback(void *callbackParam, uint8_t *isStillValid)
{
    /* if pre contract exist, return true */
    *isStillValid = 1;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PRINTF("partner sink's request %dV success\r\n",
               (pdAppInstance->sinkRequestRDO.bitFields.objectPosition == 1) ? 5 : 9);
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PRINTF("source has error in power negotiation\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSrcCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint32_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_source_pdo_t sourcePDO;
    PRINTF("receive source capabilities:\r\n");
    pdAppInstance->partnerSourceCapNumber = caps->capabilitiesCount;
    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        pdAppInstance->partnerSourceCaps[index].PDOValue = caps->capabilities[index];
        sourcePDO.PDOValue                               = caps->capabilities[index];
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PRINTF("%d: fixed PDO; ", index + 1);
                PRINTF("vol:%dmV, current:%dmA\r\n", sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT,
                       sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Variable:
            {
                PRINTF("%d: variable PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, current:%dmA\r\n", sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                       sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                       sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Battery:
            {
                PRINTF("%d: battery PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, power:%dmW\r\n", sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                       sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                       sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT);
                break;
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSrcCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("get src cap fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("get src cap replying not supported");
            break;

        default:
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGetRequestRDOCallback(void *callbackParam, pd_rdo_t *rdo)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint32_t voltage        = 0;

    if (PD_DemoFindPDO(pdAppInstance, rdo, PD_DEMO_EXPECTED_VOLTAGE, PD_DEMO_EXPECTED_CURRENT, &voltage) !=
        kStatus_PD_Success)
    {
        PRINTF("cap mismatch\r\n");
    }
    pdAppInstance->sinkRequestRDO     = *rdo;
    pdAppInstance->sinkRequestVoltage = voltage;

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PRINTF("sink request %dV success\r\n", pdAppInstance->sinkRequestVoltage / 1000);
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PRINTF("sink request power result: fail\r\n");
                break;

            case kCommandResult_Reject:
                PRINTF("sink request power result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("sink request power result: wait\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    if (success)
    {
        PRINTF("success\r\n");
    }
    else
    {
        PRINTF("fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    if (success)
    {
        PRINTF("success\r\n");
    }
    else
    {
        PRINTF("fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSnkCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint32_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_sink_pdo_t sinkPDO;
    PRINTF("receive sink capabilities:\r\n");
    pdAppInstance->partnerSinkCapNumber = caps->capabilitiesCount;
    for (index = 0; index < pdAppInstance->partnerSinkCapNumber; ++index)
    {
        pdAppInstance->partnerSinkCaps[index].PDOValue = caps->capabilities[index];
        sinkPDO.PDOValue                               = caps->capabilities[index];
        switch (sinkPDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PRINTF("%d: fixed PDO; ", index + 1);
                PRINTF("vol:%dmV, current:%dmA\r\n", sinkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT,
                       sinkPDO.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Variable:
            {
                PRINTF("%d: variable PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, current:%dmA\r\n", sinkPDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                       sinkPDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                       sinkPDO.variablePDO.operateCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Battery:
            {
                PRINTF("%d: battery PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, power:%dmW\r\n", sinkPDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                       sinkPDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                       sinkPDO.batteryPDO.operatePower * PD_PDO_POWER_UNIT);
                break;
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSnkCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("get snk cap fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("get snk cap replying not supported");
            break;

        default:
            break;
    }

    return kStatus_PD_Success;
}
