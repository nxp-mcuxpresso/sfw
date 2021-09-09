/*
 * Copyright 2016 - 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_cmsis_wrapper.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"

#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum _cmsis_driver_state
{
    CMSIS_IDLE,
    CMSIS_TRANSFERING,
    CMSIS_TRANSFER_ERROR_DONE,
} cmsis_driver_state_t;

typedef struct _cmsis_i2c_driver_adapter
{
    void *cmsisInterface;
    void *callback;
    /* uint16_t i2cAddress; */
    uint8_t occupied;
    uint8_t interface;
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    volatile uint8_t dataBuffer[140];
#else
    volatile uint8_t dataBuffer[64];
#endif
    volatile uint8_t cmsisState;
    volatile uint8_t cmsisResult;
    volatile uint8_t cmsisTry;
} cmsis_i2c_adapter_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 0)
static void CMSIS_SignalEvent0(uint32_t event);
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 1)
static void CMSIS_SignalEvent1(uint32_t event);
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 2)
static void CMSIS_SignalEvent2(uint32_t event);
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 3)
static void CMSIS_SignalEvent3(uint32_t event);
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 4)
static void CMSIS_SignalEvent4(uint32_t event);
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 5)
static void CMSIS_SignalEvent5(uint32_t event);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if defined(RTE_I2C0) && (RTE_I2C0)
extern ARM_DRIVER_I2C Driver_I2C0;
#endif
#if defined(RTE_I2C1) && (RTE_I2C1)
extern ARM_DRIVER_I2C Driver_I2C1;
#endif
#if defined(RTE_I2C2) && (RTE_I2C2)
extern ARM_DRIVER_I2C Driver_I2C2;
#endif
#if defined(RTE_I2C3) && (RTE_I2C3)
extern ARM_DRIVER_I2C Driver_I2C3;
#endif
#if defined(RTE_I2C4) && (RTE_I2C4)
extern ARM_DRIVER_I2C Driver_I2C4;
#endif
#if defined(RTE_I2C5) && (RTE_I2C5)
extern ARM_DRIVER_I2C Driver_I2C5;
#endif
#if defined(RTE_I2C6) && (RTE_I2C6)
extern ARM_DRIVER_I2C Driver_I2C6;
#endif
#if defined(RTE_I2C7) && (RTE_I2C7)
extern ARM_DRIVER_I2C Driver_I2C7;
#endif
#if defined(RTE_I2C8) && (RTE_I2C8)
extern ARM_DRIVER_I2C Driver_I2C8;
#endif
#if defined(RTE_I2C9) && (RTE_I2C9)
extern ARM_DRIVER_I2C Driver_I2C9;
#endif

const ARM_I2C_SignalEvent_t s_CMSISI2CDriverCallback[CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT] = {
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 0)
    CMSIS_SignalEvent0,
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 1)
    CMSIS_SignalEvent1,
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 2)
    CMSIS_SignalEvent2,
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 3)
    CMSIS_SignalEvent3,
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 4)
    CMSIS_SignalEvent4,
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 5)
    CMSIS_SignalEvent5,
#endif
};
static cmsis_i2c_adapter_t s_CMSISI2CDrvInstance[CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT];
#if (PD_CONFIG_MAX_PORT > CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT)
#error "CMSIS driver error, please increase the wrapper instance count"
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

void *CMSIS_GetI2cInterface(uint8_t interface)
{
    ARM_DRIVER_I2C *driverI2CArray[] = {
#if defined(RTE_I2C0) && (RTE_I2C0)
        &Driver_I2C0,
#else
        NULL,
#endif
#if defined(RTE_I2C1) && (RTE_I2C1)
        &Driver_I2C1,
#else
        NULL,
#endif
#if defined(RTE_I2C2) && (RTE_I2C2)
        &Driver_I2C2,
#else
        NULL,
#endif
#if defined(RTE_I2C3) && (RTE_I2C3)
        &Driver_I2C3,
#else
        NULL,
#endif
#if defined(RTE_I2C4) && (RTE_I2C4)
        &Driver_I2C4,
#else
        NULL,
#endif
#if defined(RTE_I2C5) && (RTE_I2C5)
        &Driver_I2C5,
#else
        NULL,
#endif
#if defined(RTE_I2C6) && (RTE_I2C6)
        &Driver_I2C6,
#else
        NULL,
#endif
#if defined(RTE_I2C7) && (RTE_I2C7)
        &Driver_I2C7,
#else
        NULL,
#endif
#if defined(RTE_I2C8) && (RTE_I2C8)
        &Driver_I2C8,
#else
        NULL,
#endif
#if defined(RTE_I2C9) && (RTE_I2C9)
        &Driver_I2C9,
#else
        NULL,
#endif
    };

    if ((interface - kInterface_i2c0 + 1) > (sizeof(driverI2CArray) / 4))
    {
        return NULL;
    }
    else
    {
        return (void *)(driverI2CArray[interface - kInterface_i2c0]);
    }
}

#if defined(RTE_I2C0) && (RTE_I2C0)
USB_WEAK_FUN void BOARD_I2C0_ReleaseBus(void)
{
}
#endif
#if defined(RTE_I2C1) && (RTE_I2C1)
USB_WEAK_FUN void BOARD_I2C1_ReleaseBus(void)
{
}
#endif
#if defined(RTE_I2C2) && (RTE_I2C2)
USB_WEAK_FUN void BOARD_I2C2_ReleaseBus(void)
{
}
#endif
#if defined(RTE_I2C3) && (RTE_I2C3)
USB_WEAK_FUN void BOARD_I2C3_ReleaseBus(void)
{
}
#endif
#if defined(RTE_I2C4) && (RTE_I2C4)
USB_WEAK_FUN void BOARD_I2C4_ReleaseBus(void)
{
}
#endif
#if defined(RTE_I2C5) && (RTE_I2C5)
USB_WEAK_FUN void BOARD_I2C5_ReleaseBus(void)
{
}
#endif

void CMSIS_I2cReleaseBus(uint8_t interface)
{
    switch (interface)
    {
#if defined(RTE_I2C0) && (RTE_I2C0)
        case kInterface_i2c0:
            BOARD_I2C0_ReleaseBus();
            break;
#endif
#if defined(RTE_I2C1) && (RTE_I2C1)
        case kInterface_i2c1:
            BOARD_I2C1_ReleaseBus();
            break;
#endif
#if defined(RTE_I2C2) && (RTE_I2C2)
        case kInterface_i2c2:
            BOARD_I2C2_ReleaseBus();
            break;
#endif
#if defined(RTE_I2C3) && (RTE_I2C3)
        case kInterface_i2c3:
            BOARD_I2C3_ReleaseBus();
            break;
#endif
#if defined(RTE_I2C4) && (RTE_I2C4)
        case kInterface_i2c4:
            BOARD_I2C4_ReleaseBus();
            break;
#endif
#if defined(RTE_I2C5) && (RTE_I2C5)
        case kInterface_i2c5:
            BOARD_I2C5_ReleaseBus();
            break;
#endif
        default:
            break;
    }
}

static void CMSIS_SignalEventCommon(uint8_t index, uint32_t event)
{
    if (s_CMSISI2CDrvInstance[index].cmsisState != CMSIS_TRANSFERING)
    {
        return;
    }

    switch (event)
    {
        case ARM_I2C_EVENT_TRANSFER_DONE:
            s_CMSISI2CDrvInstance[index].cmsisState = CMSIS_IDLE;
            break;

        case ARM_I2C_EVENT_TRANSFER_INCOMPLETE:
        case ARM_I2C_EVENT_ADDRESS_NACK:
        case ARM_I2C_EVENT_ARBITRATION_LOST:
        case ARM_I2C_EVENT_BUS_ERROR:
        case ARM_I2C_EVENT_BUS_CLEAR:
        case ARM_I2C_EVENT_SLAVE_TRANSMIT:
        case ARM_I2C_EVENT_SLAVE_RECEIVE:
        case ARM_I2C_EVENT_GENERAL_CALL:
        default:
            s_CMSISI2CDrvInstance[index].cmsisState = CMSIS_TRANSFER_ERROR_DONE;
            break;
    }
}

#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 0)
static void CMSIS_SignalEvent0(uint32_t event)
{
    CMSIS_SignalEventCommon(0, event);
}
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 1)
static void CMSIS_SignalEvent1(uint32_t event)
{
    CMSIS_SignalEventCommon(1, event);
}
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 2)
static void CMSIS_SignalEvent2(uint32_t event)
{
    CMSIS_SignalEventCommon(2, event);
}
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 3)
static void CMSIS_SignalEvent3(uint32_t event)
{
    CMSIS_SignalEventCommon(3, event);
}
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 4)
static void CMSIS_SignalEvent4(uint32_t event)
{
    CMSIS_SignalEventCommon(4, event);
}
#endif
#if defined(CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT) && (CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT > 5)
static void CMSIS_SignalEvent5(uint32_t event)
{
    CMSIS_SignalEventCommon(5, event);
}
#endif

int32_t CMSIS_I2CInterfaceInit(void **cmsisI2CDriver, uint8_t interface, void *interfaceConfig)
{
    uint8_t index = 0;
    int32_t status;
    /* pd_i2c_interface_config_t *i2cConfig = (pd_i2c_interface_config_t *)interfaceConfig; */
    void *cmsisInterface                 = NULL;
    cmsis_i2c_adapter_t *cmsisI2cAdapter = NULL;
    OSA_SR_ALLOC();

    cmsisInterface = CMSIS_GetI2cInterface(interface);
    if (cmsisInterface == NULL)
    {
        return ARM_DRIVER_ERROR;
    }

    OSA_ENTER_CRITICAL();
    for (; index < CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT; index++)
    {
        if (s_CMSISI2CDrvInstance[index].occupied != 1)
        {
            uint8_t *buffer = (uint8_t *)&s_CMSISI2CDrvInstance[index];
            for (uint32_t j = 0U; j < sizeof(cmsis_i2c_adapter_t); j++)
            {
                buffer[j] = 0x00U;
            }
            s_CMSISI2CDrvInstance[index].occupied = 1;
            cmsisI2cAdapter                       = &s_CMSISI2CDrvInstance[index];
            break;
        }
    }
    OSA_EXIT_CRITICAL();
    if (cmsisI2cAdapter == NULL)
    {
        return ARM_DRIVER_ERROR;
    }

    cmsisI2cAdapter->interface = interface;
    cmsisI2cAdapter->callback  = (void *)s_CMSISI2CDriverCallback[index];
    /* cmsis->i2cAddress = i2cConfig->slaveAddress; */
    cmsisI2cAdapter->cmsisState     = CMSIS_IDLE;
    cmsisI2cAdapter->cmsisInterface = cmsisInterface;

    status = ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))
                 ->Initialize((ARM_I2C_SignalEvent_t)cmsisI2cAdapter->callback);
    if (status == ARM_DRIVER_OK)
    {
        status = ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->PowerControl(ARM_POWER_FULL);
    }
    if (status == ARM_DRIVER_OK)
    {
        /*config transmit speed*/
        status =
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
    }

    *cmsisI2CDriver = cmsisI2cAdapter;

    return status;
}

int32_t CMSIS_I2CInterfaceDeinit(void *cmsisI2CDriver)
{
    int32_t status;
    cmsis_i2c_adapter_t *cmsisI2cAdapter = (cmsis_i2c_adapter_t *)cmsisI2CDriver;
    OSA_SR_ALLOC();

    status = ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->Uninitialize();
    OSA_ENTER_CRITICAL();
    cmsisI2cAdapter->occupied = 0;
    OSA_EXIT_CRITICAL();
    return status;
}

static int32_t CMSIS_I2CInterfaceTransfer(
    cmsis_i2c_adapter_t *cmsisI2cAdapter, uint32_t slave, uint8_t send, uint8_t *data, uint32_t num, uint8_t pending)
{
    int32_t result = ARM_DRIVER_ERROR;
    uint32_t waitCount;

    cmsisI2cAdapter->cmsisState = CMSIS_TRANSFERING;
    if (send)
    {
        result = ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))
                     ->MasterTransmit(slave, (const uint8_t *)data, num, pending);
    }
    else
    {
        result = ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->MasterReceive(slave, data, num, pending);
    }

    if (result == ARM_DRIVER_OK)
    {
        waitCount = 1000000;
        while (cmsisI2cAdapter->cmsisState == CMSIS_TRANSFERING)
        {
            waitCount--;
            if (waitCount == 0)
            {
                cmsisI2cAdapter->cmsisState = CMSIS_TRANSFER_ERROR_DONE;
                break;
            }
        }
        if (cmsisI2cAdapter->cmsisState == CMSIS_IDLE)
        {
            result = ARM_DRIVER_OK;
        }
        else
        {
            result = ARM_DRIVER_ERROR;
        }
    }

    return result;
}

int32_t CMSIS_I2CInterfaceWriteRegister(
    void *cmsisI2CDriver, uint32_t slave, uint32_t registerAddr, uint8_t registerLen, const uint8_t *data, uint32_t num)
{
    int32_t result;
    cmsis_i2c_adapter_t *cmsisI2cAdapter = (cmsis_i2c_adapter_t *)cmsisI2CDriver;
    uint8_t index;
    uint8_t *dataPtr = (uint8_t *)data;

    cmsisI2cAdapter->cmsisTry = CMSIS_TRANSFER_RETRY_COUNT;
    if (registerLen > 0)
    {
        if (registerLen == 1)
        {
            cmsisI2cAdapter->dataBuffer[0] = registerAddr;
        }
        else
        {
            cmsisI2cAdapter->dataBuffer[0] = registerAddr;
            cmsisI2cAdapter->dataBuffer[1] = (registerAddr >> 8);
            registerLen                    = 2;
        }
        for (index = 0; index < num; ++index)
        {
            cmsisI2cAdapter->dataBuffer[registerLen + index] = data[index];
        }
        dataPtr = (uint8_t *)&(cmsisI2cAdapter->dataBuffer[0]);
    }

    do
    {
        result = ARM_DRIVER_ERROR;
        result = CMSIS_I2CInterfaceTransfer(cmsisI2cAdapter, slave, 1, dataPtr, registerLen + num, 0);

        if (result == ARM_DRIVER_OK)
        {
            break;
        }
        else
        {
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
            CMSIS_I2cReleaseBus(cmsisI2cAdapter->interface);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))
                ->Initialize((ARM_I2C_SignalEvent_t)cmsisI2cAdapter->callback);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->PowerControl(ARM_POWER_OFF);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->PowerControl(ARM_POWER_FULL);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
        }
    } while (--cmsisI2cAdapter->cmsisTry);

    return result;
}

int32_t CMSIS_I2CInterfaceReadRegister(
    void *cmsisI2CDriver, uint32_t slave, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num)
{
    int32_t result;
    cmsis_i2c_adapter_t *cmsisI2cAdapter = (cmsis_i2c_adapter_t *)cmsisI2CDriver;

    cmsisI2cAdapter->cmsisTry = CMSIS_TRANSFER_RETRY_COUNT;
    do
    {
        result = ARM_DRIVER_ERROR;
        result = CMSIS_I2CInterfaceTransfer(cmsisI2cAdapter, slave, 1, (uint8_t *)&registerAddr, registerLen, 1);
        if (result == ARM_DRIVER_OK)
        {
            result = CMSIS_I2CInterfaceTransfer(cmsisI2cAdapter, slave, 0, data, num, 0);
        }

        if (result == ARM_DRIVER_OK)
        {
            break;
        }
        else
        {
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
            CMSIS_I2cReleaseBus(cmsisI2cAdapter->interface);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))
                ->Initialize((ARM_I2C_SignalEvent_t)cmsisI2cAdapter->callback);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->PowerControl(ARM_POWER_OFF);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->PowerControl(ARM_POWER_FULL);
            ((ARM_DRIVER_I2C *)(cmsisI2cAdapter->cmsisInterface))->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
        }
    } while (--cmsisI2cAdapter->cmsisTry);

    return result;
}

#endif
