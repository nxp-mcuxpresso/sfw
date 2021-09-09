/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "string.h"
#include "pd_board_config.h"
#include "usb_pd_alt_mode.h"
#include "usb_pd_alt_mode_dp.h"
#include "pd_dp_hpd_driver.h"
#include "pd_typec_crossbar.h"
#include "pd_app_misc.h"
#include "pd_dp_board_chip.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _pd_board_chip_instance
{
    pd_crossbar_instance_t crossbarInstance;
    pd_hpd_driver_t hpdDriverInstance;
    pd_handle pdHandle;
    uint8_t occupied;
} pd_board_chip_instance_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_board_chip_instance_t s_PDBoardChipInstances[PD_CONFIG_MAX_PORT];

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PD_DpBoardChipsInit(void **interfaceHandle, pd_handle pdHandle, const void *param)
{
    uint32_t index                           = 0;
    pd_board_chip_instance_t *pdChipInstance = NULL;
    APP_CRITICAL_ALLOC();

    APP_ENTER_CRITICAL();
    for (index = 0; index < sizeof(s_PDBoardChipInstances) / sizeof(pd_board_chip_instance_t); ++index)
    {
        if (s_PDBoardChipInstances[index].occupied == 0)
        {
            s_PDBoardChipInstances[index].occupied = 1;
            pdChipInstance                         = &s_PDBoardChipInstances[index];
            break;
        }
    }

    if (pdChipInstance == NULL)
    {
        APP_EXIT_CRITICAL();
        return kStatus_PD_Error;
    }
    APP_EXIT_CRITICAL();

    pdChipInstance->pdHandle = pdHandle;
    if (PD_CrossbarInit(&pdChipInstance->crossbarInstance, pdChipInstance->pdHandle,
                        ((pd_dp_board_chip_config_t *)param)->crossbarConfig) != 0)
    {
        pdChipInstance->occupied = 0;
        return kStatus_PD_Error;
    }

    if (PD_DpHpdDriverInit(&pdChipInstance->hpdDriverInstance, pdChipInstance,
                           ((pd_dp_board_chip_config_t *)param)->hpdDriverConfig) != 0)
    {
        PD_CrossbarDeinit(&pdChipInstance->crossbarInstance);
        pdChipInstance->occupied = 0;
        return kStatus_PD_Error;
    }

    *interfaceHandle = pdChipInstance;
    return kStatus_PD_Success;
}

pd_status_t PD_DpBoardChipsDeinit(void *interfaceHandle)
{
    pd_board_chip_instance_t *pdChipInstance = (pd_board_chip_instance_t *)interfaceHandle;
    if (interfaceHandle != NULL)
    {
        PD_CrossbarDeinit(&pdChipInstance->crossbarInstance);
        PD_DpHpdDriverDeinit(&pdChipInstance->hpdDriverInstance);
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpBoardChipsControl(void *interfaceHandle, uint32_t opCode, void *opParam)
{
    pd_board_chip_instance_t *pdChipInstance = (pd_board_chip_instance_t *)interfaceHandle;
    if (interfaceHandle == NULL)
    {
        return kStatus_PD_Error;
    }

    switch (opCode)
    {
        case kDPPeripheal_ControlHPDValue:
            PD_DpHpdDriverControl(&pdChipInstance->hpdDriverInstance, *((uint8_t *)opParam));
            break;

        case kDPPeripheal_ControlHPDSetLow:
            PD_DpHpdDriverSetLow(&pdChipInstance->hpdDriverInstance);
            break;

        case kDPPeripheal_ControlHPDReleaseLow:
            PD_DpHpdDriverReleaseLow(&pdChipInstance->hpdDriverInstance);
            break;

        case kDPPeripheal_ControlSetMuxSaftMode:
            PD_CrossbarSetMux(&pdChipInstance->crossbarInstance, MUX_SAFE_MODE, 0);
            break;

        case kDPPeripheal_ControlSetMuxUSB3Only:
            PD_CrossbarSetMux(&pdChipInstance->crossbarInstance, MUX_USB3_ONLY, 0);
            break;

        case kDPPeripheal_ControlSetMuxShutDown:
            PD_CrossbarSetMux(&pdChipInstance->crossbarInstance, MUX_SHUTDOWN, 0);
            break;

        case kDPPeripheal_ControlSetMuxDP4LANE:
            PD_CrossbarSetMux(&pdChipInstance->crossbarInstance, MUX_DP4_LANE, *((uint32_t *)opParam));
            break;

        case kDPPeripheal_ControlSetMuxDP2LANEUSB3:
            PD_CrossbarSetMux(&pdChipInstance->crossbarInstance, MUX_DP2_LANE_USB3, *((uint32_t *)opParam));
            break;

        case kDPPeripheal_ControlSetMuxDisable:
            PD_CrossbarSetMux(&pdChipInstance->crossbarInstance, MUX_DISABLED, 0);
            break;

        default:
            break;
    }

    return kStatus_PD_Success;
}

void PD_DpBoardChip1msIsr(void)
{
    for (uint8_t index = 0; index < sizeof(s_PDBoardChipInstances) / sizeof(pd_board_chip_instance_t); ++index)
    {
        if (s_PDBoardChipInstances[index].occupied)
        {
            PD_DpHpdDriver1msISR(&s_PDBoardChipInstances[index].hpdDriverInstance);
        }
    }
}

void PD_DpBoardChipTask(void)
{
    for (uint8_t index = 0; index < sizeof(s_PDBoardChipInstances) / sizeof(pd_board_chip_instance_t); ++index)
    {
        if (s_PDBoardChipInstances[index].occupied)
        {
            PD_DpHpdDrvierProcess(&s_PDBoardChipInstances[index].hpdDriverInstance);
        }
    }
}
