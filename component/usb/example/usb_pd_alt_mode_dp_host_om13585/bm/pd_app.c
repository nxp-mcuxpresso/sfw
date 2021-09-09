/*
 * Copyright 2016 - 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "string.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "board.h"
#include "pd_power_interface.h"
#include "pd_board_config.h"
#include "pd_dp_board_chip.h"
#include "pd_power_nx20p3483.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (PD_DEMO_PORTS_COUNT > PD_CONFIG_MAX_PORT)
#error "please increase the instance count"
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void BOARD_InitHardware(void);
void HW_GpioInit(uint8_t port);
void HW_TimerInit(void);
void HW_GpioPDPHYIntEnable(uint8_t port, uint8_t enable);
void HW_TimerCallback(void);
pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
pd_app_t g_PDAppInstancePort1;
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
pd_app_t g_PDAppInstancePort2;
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
pd_app_t g_PDAppInstancePort3;
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
pd_app_t g_PDAppInstancePort4;
#endif

extern pd_instance_config_t *g_PortsConfigArray[];
pd_app_t *g_PDAppInstanceArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
    &g_PDAppInstancePort1,
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
    &g_PDAppInstancePort2,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
    &g_PDAppInstancePort3,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
    &g_PDAppInstancePort4,
#endif
};

pd_power_handle_callback_t callbackFunctions = {
    PD_PowerSrcTurnOnDefaultVbus,  PD_PowerSrcTurnOnRequestVbus,  PD_PowerSrcTurnOffVbus,
    PD_PowerSrcGotoMinReducePower, PD_PowerSnkDrawTypeCVbus,      PD_PowerSnkDrawRequestVbus,
    PD_PowerSnkStopDrawVbus,       PD_PowerSnkGotoMinReducePower, PD_PowerControlVconn,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

void PD_BMEnterCritical(uint32_t *sr)
{
    *sr = DisableGlobalIRQ();
    __ASM("CPSID i");
}

void PD_BMExitCritical(uint32_t sr)
{
    EnableGlobalIRQ(sr);
}

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
void HW_GpioPDPHYPort1IntCallback(void)
{
    PD_PTN5110IsrFunction(g_PDAppInstancePort1.pdHandle);
}
#endif

#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
void HW_GpioPDPHYPort2IntCallback(void)
{
    PD_PTN5110IsrFunction(g_PDAppInstancePort2.pdHandle);
}
#endif

#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
void HW_GpioPDPHYPort3IntCallback(void)
{
    PD_PTN5110IsrFunction(g_PDAppInstancePort3.pdHandle);
}
#endif

#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
void HW_GpioPDPHYPort4IntCallback(void)
{
    PD_PTN5110IsrFunction(g_PDAppInstancePort4.pdHandle);
}
#endif

uint8_t PD_AppGetBoardRev(uint8_t port)
{
    if ((port == 0) || (port > PD_CONFIG_MAX_PORT))
    {
        return 0;
    }
    else
    {
        pd_app_t *pdAppInstanceArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
            &g_PDAppInstancePort1,
#else
            NULL,
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
            &g_PDAppInstancePort2,
#else
            NULL,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
            &g_PDAppInstancePort3,
#else
            NULL,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
            &g_PDAppInstancePort4,
#else
            NULL,
#endif
        };

        if (pdAppInstanceArray[port - 1]->portShieldVersion == 0)
        {
            if (PD_NX20PCheckExist(port))
            {
                pdAppInstanceArray[port - 1]->portShieldVersion = 2;
            }
            else
            {
                pdAppInstanceArray[port - 1]->portShieldVersion = 1;
            }
        }
        return pdAppInstanceArray[port - 1]->portShieldVersion;
    }
}

pd_status_t PD_DpmConnectCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status      = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_DISCONNECTED:
            PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
            PRINTF("port %d disconnect\r\n", pdAppInstance->portNumber);
            status = kStatus_PD_Success;
            break;

        case PD_CONNECT_ROLE_CHANGE:
        case PD_CONNECTED:
        {
            uint8_t getInfo;

            pdAppInstance->partnerSourceCapNumber = 0;
            pdAppInstance->partnerSinkCapNumber   = 0;
            pdAppInstance->portShieldVersion      = 0;
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &getInfo);

            PRINTF((event == PD_CONNECTED) ? "port %d connected," : "port %d connect change,",
                   pdAppInstance->portNumber);
            PRINTF(" power role:%s,", (getInfo == kPD_PowerRoleSource) ? "Source" : "Sink");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &getInfo);
            PRINTF(" data role:%s,", (getInfo == kPD_DataRoleDFP) ? "DFP" : "UFP");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &getInfo);
            PRINTF(" vconn source:%s\r\n", (getInfo == kPD_IsVconnSource) ? "yes" : "no");
            status = kStatus_PD_Success;
            break;
        }

        default:
            break;
    }

    return status;
}

pd_status_t PD_StackDemoAppCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status      = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_FUNCTION_DISABLED:
            /* need hard or software reset */
            status = kStatus_PD_Success;
            break;

        case PD_CONNECTED:
        case PD_CONNECT_ROLE_CHANGE:
        case PD_DISCONNECTED:
            status = PD_DpmConnectCallback(callbackParam, event, param);
            break;

        case PD_DPM_ALTMODE_DP_DFP_MODE_CONFIGURED:
            PRINTF("enter and configure DP\r\n");
            break;

        case PD_DPM_ALTMODE_DP_DFP_MODE_UNCONFIGURED:
            PRINTF("Exit DP\r\n");
            break;

        case PD_DPM_OVP_OCP_FAULT:
            if (PD_AppGetBoardRev(pdAppInstance->portNumber) == 2)
            {
                PD_NX20PClearInt(pdAppInstance->portNumber);
            }
            break;

        case PD_DPM_VBUS_ALARM:
            /* Users need to take care of this. Some corrective actions may be taken, such as disconnect
               or debounce time to turn off the power switch. It is up to the system power management. */
            break;

        default:
            status = PD_DpmAppCommandCallback(callbackParam, event, param);
            break;
    }
    return status;
}

void PD_AppInit(void)
{
    uint8_t index;
    pd_app_t *pdAppInstance;
    pd_app_t *pdAppInstanceArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
        &g_PDAppInstancePort1,
#else
        NULL,
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
        &g_PDAppInstancePort2,
#else
        NULL,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
        &g_PDAppInstancePort3,
#else
        NULL,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
        &g_PDAppInstancePort4,
#else
        NULL,
#endif
    };

    for (index = 0; index < 4; ++index)
    {
        if (pdAppInstanceArray[index] != NULL)
        {
            pdAppInstanceArray[index]->portNumber = (index + 1);
            HW_GpioInit((index + 1));
        }
    }

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        pdAppInstance                = g_PDAppInstanceArray[index];
        pdAppInstance->pdHandle      = NULL;
        pdAppInstance->pdConfigParam = g_PortsConfigArray[index];
        PD_I2cInit(&((phy_interface_param_t *)pdAppInstance->pdConfigParam->phyInterfaceParam)->i2cHandle,
                   kInterface_i2c0 + BOARD_PD_I2C_INDEX, HW_I2CGetFreq, HW_I2CReleaseBus);
        if (PD_InstanceInit(&pdAppInstance->pdHandle, PD_StackDemoAppCallback, &callbackFunctions, pdAppInstance,
                            g_PortsConfigArray[index]) != kStatus_PD_Success)
        {
            PRINTF("pd port %d init fail\r\n", pdAppInstance->portNumber);
        }
        else
        {
            PD_PowerBoardControlInit(pdAppInstance->portNumber, pdAppInstance->pdHandle);

            pdAppInstance->msgSop                 = kPD_MsgSOP;
            pdAppInstance->partnerSourceCapNumber = 0;
            pdAppInstance->partnerSinkCapNumber   = 0;
            /* evaluate result */
            pdAppInstance->prSwapAccept    = 1;
            pdAppInstance->vconnSwapAccept = 1;
            PRINTF("pd port %d init success\r\n", pdAppInstance->portNumber);
        }
    }
}

void HW_TimerCallback(void)
{
    /* Callback into timer service */
    uint8_t index;
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        PD_TimerIsrFunction(g_PDAppInstanceArray[index]->pdHandle);
    }
    PD_DpBoardChip1msIsr();
}

int main(void)
{
    uint8_t index;
    BOARD_InitHardware();

    HW_TimerInit();
    PD_AppInit();

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index]->pdHandle != NULL)
        {
            HW_GpioPDPHYIntEnable(g_PDAppInstanceArray[index]->portNumber, 1);
        }
    }

    while (1)
    {
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
        PD_Task();
#else
        for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            PD_InstanceTask(g_PDAppInstanceArray[index]->pdHandle);
        }
#endif
        PD_AltModeTask();
        PD_DpBoardChipTask();
    }
}
