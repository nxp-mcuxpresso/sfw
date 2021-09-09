/*
 * Copyright 2016 - 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "string.h"
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "pd_i2c_over_vdm.h"
#include "pd_command_interface.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "board.h"
#include "pd_power_interface.h"
#include "pd_board_config.h"
#include "timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (PD_DEMO_PORTS_COUNT > PD_CONFIG_MAX_PORT)
#error "please increase the instance count"
#endif

#ifndef PD_TIMER_INSTANCE
#define PD_TIMER_INSTANCE (0)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

uint32_t HW_I2CGetFreq(uint8_t i2cInstance);
void HW_I2CReleaseBus(void);
void PD_DemoInit(void);
void BOARD_InitHardware(void);
uint32_t HW_TimerGetFreq(void);
pd_status_t PD_PowerBoardReset(uint8_t port);
pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param);
void PD_DemoSoftTimer1msProcess(void);
void PD_DemoReset(pd_app_t *pdAppInstance);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_source_pdo_t s_PortSourceCaps[] = {
    {
        /* PDO1: fixed supply: dual-role power; Externally Powered; no USB communication; dual-role data; 5V; 3A */
        .PDOValue                 = 0,
        .fixedPDO.dualRoleData    = 1,
        .fixedPDO.dualRolePower   = 1,
        .fixedPDO.externalPowered = 1,
        .fixedPDO.fixedSupply     = kPDO_Fixed,
        .fixedPDO.maxCurrent      = (3 * 100),
        .fixedPDO.peakCurrent     = 0,
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
        .fixedPDO.unchunkedSupported = 1,
#endif
        .fixedPDO.usbCommunicationsCapable = 0,
        .fixedPDO.usbSuspendSupported      = 0,
        .fixedPDO.voltage                  = (5 * 1000 / 50),
    },
    {
        /* PDO2: fixed Supply: 9V - 2A */
        .PDOValue             = 0,
        .fixedPDO.fixedSupply = kPDO_Fixed,
        .fixedPDO.maxCurrent  = (2 * 100),
        .fixedPDO.voltage     = (9 * 1000 / 50),
    },
};

static pd_power_port_config_t s_PortPowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    NULL,                                               /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    0,                                                  /* sink cap count */
    kPowerConfig_SourceOnly,                            /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_None,                                     /* drp try function */
    kDataConfig_DFP,                                    /* data function */
    1,                                                  /* support vconn */
    0,                                                  /* reserved */
    NULL,
    NULL,
    NULL,
};

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
static pd_phy_config_t s_Port1PhyConfig = {
    .i2cInstance   = kInterface_i2c0 + BOARD_PD_I2C_INDEX,
    .slaveAddress  = 0x50u,
    .i2cSrcClock   = 0u,
    .i2cReleaseBus = HW_I2CReleaseBus,
    .alertPort     = PD_PORT1_PHY_INTERRUPT_PORT,
    .alertPin      = PD_PORT1_PHY_INTERRUPT_PIN,
    .alertPriority = PD_PORT1_PHY_INTERRUPT_PRIORITY,
};

pd_instance_config_t g_Port1PDConfig = {
    kDeviceType_NormalPowerPort, /* normal power port */
    kPD_PhyPTN5110,
    &s_Port1PhyConfig,
    &s_PortPowerConfig,
};
#endif

pd_instance_config_t *g_PortsConfigArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
    &g_Port1PDConfig,
#endif
};

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
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
    &g_PDAppInstancePort4,
#endif
};

pd_power_handle_callback_t callbackFunctions = {
    PD_PowerSrcTurnOnDefaultVbus,
    PD_PowerSrcTurnOnRequestVbus,
    PD_PowerSrcTurnOffVbus,
    PD_PowerSrcGotoMinReducePower,
    NULL,
    NULL,
    NULL,
    NULL,
    PD_PowerControlVconn,
};

TIMER_HANDLE_DEFINE(g_PDTimerHandle);
/*******************************************************************************
 * Code
 ******************************************************************************/

void PD_GpioInit(pd_app_t *pdAppInstance)
{
    pd_demo_io_init_t portsDemoPinConfigArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
        {
            .extraEnSrcPort = PD_PORT1_EXTRA_SRC_PORT,
            .extraEnSrcPin  = PD_PORT1_EXTRA_SRC_PIN,
        },
#endif
    };
    pd_demo_io_init_t *demoGpioPinConfig = &portsDemoPinConfigArray[pdAppInstance->portNumber - 1];
    hal_gpio_pin_config_t config;

    config.direction = kHAL_GpioDirectionOut;
    config.port      = demoGpioPinConfig->extraEnSrcPort;
    config.pin       = demoGpioPinConfig->extraEnSrcPin;
    config.level     = 0;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioExtraSrcHandle[0]), &config);
}

pd_status_t PD_DpmConnectCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status      = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_DISCONNECTED:
            PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
            PD_DemoReset(pdAppInstance);
            PRINTF("port %d disconnect\r\n", pdAppInstance->portNumber);
            status = kStatus_PD_Success;
            break;

        case PD_CONNECT_ROLE_CHANGE:
        case PD_CONNECTED:
        {
            uint8_t roleInfo;

            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &roleInfo);
            if (roleInfo == kPD_PowerRoleSource)
            {
                PRINTF("port %d provide default 5V vbus\r\n", pdAppInstance->portNumber);
            }
            PD_DemoReset(pdAppInstance);

            status = kStatus_PD_Success;
            break;
        }

        default:
            break;
    }

    return status;
}

pd_status_t PD_DpmDemoAppCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status = kStatus_PD_Error;

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
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
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
        }
    }

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        pdAppInstance                = g_PDAppInstanceArray[index];
        pdAppInstance->pdHandle      = NULL;
        pdAppInstance->pdConfigParam = g_PortsConfigArray[index];
        ((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cSrcClock =
            HW_I2CGetFreq(((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cInstance);
        pdAppInstance->selfSouceCapNumber =
            ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount;
        if (PD_InstanceInit(&pdAppInstance->pdHandle, PD_DpmDemoAppCallback, &callbackFunctions, pdAppInstance,
                            g_PortsConfigArray[index]) != kStatus_PD_Success)
        {
            PRINTF("pd port %d init fail\r\n", pdAppInstance->portNumber);
        }
        else
        {
            PD_GpioInit(pdAppInstance);
            PD_PowerBoardControlInit(pdAppInstance->portNumber, pdAppInstance->pdHandle,
                                     (hal_gpio_handle_t)(&pdAppInstance->gpioExtraSrcHandle[0]));

            pdAppInstance->msgSop = kPD_MsgSOP;
            PRINTF("pd port %d init success\r\n", pdAppInstance->portNumber);
        }
    }
}

void HW_TimerCallback(void *callbackParam)
{
    /* Callback into timer service */
    uint8_t index;
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        PD_TimerIsrFunction(g_PDAppInstanceArray[index]->pdHandle);
    }
    PD_DemoSoftTimer1msProcess();
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index]->pdHandle != NULL)
        {
            PD_I2cOverVdmTimer1msIsr(g_PDAppInstanceArray[index]->i2cHandle);
        }
    }
}

void PD_AppTimerInit(void)
{
    hal_timer_config_t halTimerConfig;
    halTimerConfig.timeout     = 1000;
    halTimerConfig.srcClock_Hz = HW_TimerGetFreq();
    halTimerConfig.instance    = PD_TIMER_INSTANCE;
    HAL_TimerInit((hal_timer_handle_t)&g_PDTimerHandle[0], &halTimerConfig);

    HAL_TimerInstallCallback((hal_timer_handle_t)&g_PDTimerHandle[0], HW_TimerCallback, NULL);
    HAL_TimerEnable((hal_timer_handle_t)&g_PDTimerHandle[0]);
}

#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
void PD_PortsTask(void *arg)
{
    while (1)
    {
        PD_Task();
    }
}
#else
void PD_PortTask(void *arg)
{
    pd_app_t *pdAppInstance = (pd_app_t *)arg;
    while (1)
    {
        PD_InstanceTask(pdAppInstance->pdHandle);
    }
}
#endif

static void pd_demo_task(void *arg)
{
    PD_AppTimerInit();
    PD_AppInit();
    PD_DemoInit();

#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
    if (xTaskCreate(PD_PortsTask, "PD", (2048u) / sizeof(portSTACK_TYPE), NULL, 5, NULL) != pdPASS)
    {
        PRINTF("create task error\r\n");
    }
#else
    for (uint8_t index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (xTaskCreate(PD_PortTask, "port", (1024u + 512u) / sizeof(portSTACK_TYPE), g_PDAppInstanceArray[index], 5,
                        NULL) != pdPASS)
        {
            PRINTF("create task error\r\n");
        }
    }
#endif

    vTaskDelete(NULL); /* delete current task */
}

int main(void)
{
    BOARD_InitHardware();

    if (xTaskCreate(pd_demo_task, "demo", 1024 / sizeof(portSTACK_TYPE), NULL, 4, NULL) != pdPASS)
    {
        PRINTF("create task error\r\n");
    }

    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
