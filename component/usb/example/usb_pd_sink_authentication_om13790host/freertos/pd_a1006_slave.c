/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "pd_i2c_over_vdm.h"
#include "pd_command_interface.h"
#include "pd_app.h"
#include "pd_board_config.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

uint32_t HW_I2CGetFreq(uint8_t i2cInstance);
void HW_I2CReleaseBus(uint8_t i2cInstance);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

uint8_t PD_A1006SlaveInit(pd_app_t *pdAppInstance)
{
    pd_i2c_over_vdm_config_t config;

    config.pdHandle      = pdAppInstance->pdHandle;
    config.callback      = NULL;
    config.callbackParam = pdAppInstance;
    config.i2cInstance   = ((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cInstance;
    config.i2cSrcClock   = ((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cSrcClock;
    config.i2cReleaseBus = (void *)((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cReleaseBus;

    return PD_I2cOverVdmInit(&(pdAppInstance->i2cOvdmHandle), &config);
}

uint32_t PD_DemoStartUpGetBatteryValue(pd_app_t *pdAppInstance)
{
    return 10; /* default start up battery is 10% */
}

void PD_DemoBatteryChange(pd_app_t *pdAppInstance)
{
    if (pdAppInstance->contractValid)
    {
        if (pdAppInstance->batteryQuantity < 100)
        {
            pdAppInstance->batteryQuantity++;
        }

        /* battery charge faster */
        if (pdAppInstance->sinkRequestVoltage > 5000)
        {
            if (pdAppInstance->batteryQuantity < 100)
            {
                pdAppInstance->batteryQuantity++;
            }
        }
    }
    else
    {
        if (pdAppInstance->batteryQuantity > 0)
        {
            pdAppInstance->batteryQuantity--;
        }
    }
}

void PD_DemoReset(pd_app_t *pdAppInstance)
{
    pdAppInstance->batteryChange = 0;
    pdAppInstance->contractValid = 0;
}

void PD_DemoInit(void)
{
    uint8_t index;

    g_DemoSemaphore = NULL;
    g_DemoSemaphore = xSemaphoreCreateCounting(0x01U, 0x00U);
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        g_PDAppInstanceArray[index]->batteryQuantity = PD_DemoStartUpGetBatteryValue(g_PDAppInstanceArray[index]);
        PD_DemoReset(g_PDAppInstanceArray[index]);
        if (PD_A1006SlaveInit(g_PDAppInstanceArray[index]))
        {
            PRINTF("a1006 slave init fail\r\n");
        }
    }
}

void PD_Demo1msIsrProcess(void)
{
    portBASE_TYPE taskToWake       = pdFALSE;
    static volatile uint32_t delay = 0;
    uint8_t index;
    delay++;

    /* 2 s */
    if (delay >= 2000)
    {
        delay = 0;
        for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            PD_DemoBatteryChange(g_PDAppInstanceArray[index]);
            g_PDAppInstanceArray[index]->batteryChange = 1;
        }
        if (g_DemoSemaphore != NULL)
        {
            if (pdPASS == xSemaphoreGiveFromISR(g_DemoSemaphore, &taskToWake))
            {
                MISRAC_DISABLE
                portYIELD_FROM_ISR(taskToWake);
                MISRAC_ENABLE
            }
        }
    }
}

pd_status_t PD_DemoFindPDO(
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
                    *voltage                      = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    if (requestCurrentmA == 0u)
                    {
                        rdo->bitFields.operateValue = sourcePDO.fixedPDO.maxCurrent;
                    }
                    else
                    {
                        rdo->bitFields.operateValue = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    }
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
                    *voltage                      = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    if (requestCurrentmA == 0u)
                    {
                        rdo->bitFields.operateValue = sourcePDO.variablePDO.maxCurrent;
                    }
                    else
                    {
                        rdo->bitFields.operateValue = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    }
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
                    if (requestCurrentmA == 0u)
                    {
                        rdo->bitFields.operateValue = sourcePDO.batteryPDO.maxAllowPower;
                    }
                    else
                    {
                        rdo->bitFields.operateValue = (requestVoltagemV * requestCurrentmA) / 1000 / PD_PDO_POWER_UNIT;
                    }
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

void PD_DemoTaskFn(void)
{
    uint8_t index;
    pd_app_t *pdAppInstance;

    if (g_DemoSemaphore != NULL)
    {
        /* wait the battery change */
        if (xSemaphoreTake(g_DemoSemaphore, portMAX_DELAY) != pdTRUE)
        {
            return;
        }
    }
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        pdAppInstance = g_PDAppInstanceArray[index];
        if (pdAppInstance->batteryChange == 1)
        {
            pdAppInstance->batteryChange = 0;

            PRINTF("battery percent:%d, charge voltage:%dV\r\n", pdAppInstance->batteryQuantity,
                   (pdAppInstance->contractValid) ? (pdAppInstance->sinkRequestVoltage / 1000) : 0u);
        }
    }
}
