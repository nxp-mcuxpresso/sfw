/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "Driver_Common.h"
#include "fsl_os_abstraction.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
extern int32_t CMSIS_I2CInterfaceInit(void **cmsisI2CDriver, uint8_t interface, void *interfaceConfig);
extern int32_t CMSIS_I2CInterfaceDeinit(void *cmsisI2CDriver);
extern int32_t CMSIS_I2CInterfaceWriteRegister(void *cmsisI2CDriver,
                                               uint32_t slave,
                                               uint32_t registerAddr,
                                               uint8_t registerLen,
                                               const uint8_t *data,
                                               uint32_t num);
extern int32_t CMSIS_I2CInterfaceReadRegister(
    void *cmsisI2CDriver, uint32_t slave, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num);
#endif

#if (defined PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
extern int32_t CMSIS_SPIInterfaceInit(void **cmsisSPIDriver, uint8_t interface, void *interfaceConfig);
extern int32_t CMSIS_SPIInterfaceDeinit(void *cmsisSPIDriver);
extern int32_t CMSIS_SPIInterfaceWriteRegister(void *cmsisSPIDriver,
                                               uint32_t slave,
                                               uint32_t registerAddr,
                                               uint8_t registerLen,
                                               const uint8_t *data,
                                               uint32_t num);
extern int32_t CMSIS_SPIInterfaceReadRegister(
    void *cmsisSPIDriver, uint32_t slave, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num);
#endif

static cmsis_driver_adapter_t s_CMSISDriverInstance[CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(FSL_RTOS_FREE_RTOS)
static uint8_t CMSIS_CheckInterruptContext(void)
{
#if defined(__GIC_PRIO_BITS)
    if ((__get_CPSR() & CPSR_M_Msk) == 0x13)
#else
    if (__get_IPSR())
#endif
    {
        return 1;
    }
    return 0;
}
#endif

int32_t CMSIS_PortControlInterfaceInit(usb_cmsis_wrapper_handle *wrapperHandle,
                                       uint8_t interface,
                                       void *interfaceConfig)
{
    int32_t status                       = ARM_DRIVER_ERROR;
    cmsis_driver_adapter_t *cmsisWrapper = NULL;
    uint8_t index;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (index = 0; index < CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT; index++)
    {
        if ((s_CMSISDriverInstance[index].refCount) && (s_CMSISDriverInstance[index].interface == interface))
        {
            s_CMSISDriverInstance[index].refCount++;
            *wrapperHandle = &s_CMSISDriverInstance[index];
            OSA_EXIT_CRITICAL();
            return ARM_DRIVER_OK;
        }
    }

    for (index = 0; index < CMSIS_DRIVER_WRAPPER_INSTANCE_COUNT; index++)
    {
        if (s_CMSISDriverInstance[index].refCount == 0)
        {
            uint8_t *buffer = (uint8_t *)&s_CMSISDriverInstance[index];
            for (uint32_t j = 0U; j < sizeof(cmsis_driver_adapter_t); j++)
            {
                buffer[j] = 0x00U;
            }
            s_CMSISDriverInstance[index].refCount = 1;
            cmsisWrapper                          = &s_CMSISDriverInstance[index];
            break;
        }
    }
    OSA_EXIT_CRITICAL();
    if (cmsisWrapper == NULL)
    {
        return ARM_DRIVER_ERROR;
    }
    cmsisWrapper->interface = interface;
    *wrapperHandle          = cmsisWrapper;

#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
    if (interface < kInterface_spi0)
#endif
    {
        status = CMSIS_I2CInterfaceInit(&cmsisWrapper->cmsisDrvHandle, interface, interfaceConfig);
    }
#endif

#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
    if ((interface >= kInterface_spi0) && (interface < kInterface_end))
#endif
    {
        status = CMSIS_SPIInterfaceInit(&cmsisWrapper->cmsisDrvHandle, interface, interfaceConfig);
    }
#endif

    if ((status == ARM_DRIVER_OK) && (cmsisWrapper != NULL))
    {
        if (OSA_MutexCreate(&(cmsisWrapper->cmsisMutex)) != KOSA_StatusSuccess)
        {
            status = ARM_DRIVER_ERROR;
        }
    }

    return status;
}

int32_t CMSIS_PortControlInterfaceDeinit(usb_cmsis_wrapper_handle wrapperHandle)
{
    int32_t status                       = ARM_DRIVER_ERROR;
    cmsis_driver_adapter_t *cmsisWrapper = (cmsis_driver_adapter_t *)wrapperHandle;
    OSA_SR_ALLOC();

    if (wrapperHandle == NULL)
    {
        return status;
    }

    OSA_ENTER_CRITICAL();
    if (cmsisWrapper->refCount > 0)
    {
        cmsisWrapper->refCount--;
        if (cmsisWrapper->refCount > 0)
        {
            OSA_EXIT_CRITICAL();
            return ARM_DRIVER_OK;
        }
    }
    OSA_EXIT_CRITICAL();

#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
    if (cmsisWrapper->interface < kInterface_spi0)
#endif
    {
        status = CMSIS_I2CInterfaceDeinit(cmsisWrapper->cmsisDrvHandle);
    }
#endif

#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
    if ((cmsisWrapper->interface >= kInterface_spi0) && (cmsisWrapper->interface < kInterface_end))
#endif
    {
        status = CMSIS_SPIInterfaceDeinit(cmsisWrapper->cmsisDrvHandle);
    }
#endif

    if (cmsisWrapper->cmsisMutex != NULL)
    {
        OSA_MutexDestroy(cmsisWrapper->cmsisMutex);
    }

    return status;
}

int32_t CMSIS_PortControlInterfaceWriteRegister(usb_cmsis_wrapper_handle wrapperHandle,
                                                uint32_t slave,
                                                uint32_t registerAddr,
                                                uint8_t registerLen,
                                                const uint8_t *data,
                                                uint32_t num)
{
    int32_t status                       = ARM_DRIVER_ERROR;
    cmsis_driver_adapter_t *cmsisWrapper = (cmsis_driver_adapter_t *)wrapperHandle;
    if (wrapperHandle == NULL)
    {
        return status;
    }

    if (
#if defined(FSL_RTOS_FREE_RTOS)
        (!CMSIS_CheckInterruptContext()) &&
#endif
        (OSA_MutexLock(cmsisWrapper->cmsisMutex) != KOSA_StatusSuccess))
    {
        return ARM_DRIVER_ERROR;
    }

#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
    if (cmsisWrapper->interface < kInterface_spi0)
#endif
    {
        status =
            CMSIS_I2CInterfaceWriteRegister(cmsisWrapper->cmsisDrvHandle, slave, registerAddr, registerLen, data, num);
    }
#endif

#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
    if ((cmsisWrapper->interface >= kInterface_spi0) && (cmsisWrapper->interface < kInterface_end))
#endif
    {
        status =
            CMSIS_SPIInterfaceWriteRegister(cmsisWrapper->cmsisDrvHandle, slave, registerAddr, registerLen, data, num);
    }
#endif

    if (
#if defined(FSL_RTOS_FREE_RTOS)
        (!CMSIS_CheckInterruptContext()) &&
#endif
        (OSA_MutexUnlock(cmsisWrapper->cmsisMutex) != KOSA_StatusSuccess))
    {
        return ARM_DRIVER_ERROR;
    }

    return status;
}

int32_t CMSIS_PortControlInterfaceReadRegister(usb_cmsis_wrapper_handle wrapperHandle,
                                               uint32_t slave,
                                               uint32_t registerAddr,
                                               uint8_t registerLen,
                                               uint8_t *data,
                                               uint32_t num)
{
    int32_t status                       = ARM_DRIVER_ERROR;
    cmsis_driver_adapter_t *cmsisWrapper = (cmsis_driver_adapter_t *)wrapperHandle;
    if (wrapperHandle == NULL)
    {
        return status;
    }

    if (
#if defined(FSL_RTOS_FREE_RTOS)
        (!CMSIS_CheckInterruptContext()) &&
#endif
        (OSA_MutexLock(cmsisWrapper->cmsisMutex) != KOSA_StatusSuccess))
    {
        return ARM_DRIVER_ERROR;
    }

#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
    if (cmsisWrapper->interface < kInterface_spi0)
#endif
    {
        status =
            CMSIS_I2CInterfaceReadRegister(cmsisWrapper->cmsisDrvHandle, slave, registerAddr, registerLen, data, num);
    }
#endif

#if defined(PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
#if defined(PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
    if ((cmsisWrapper->interface >= kInterface_spi0) && (cmsisWrapper->interface < kInterface_end))
#endif
    {
        status =
            CMSIS_SPIInterfaceReadRegister(cmsisWrapper->cmsisDrvHandle, slave, registerAddr, registerLen, data, num);
    }
#endif

    if (
#if defined(FSL_RTOS_FREE_RTOS)
        (!CMSIS_CheckInterruptContext()) &&
#endif
        (OSA_MutexUnlock(cmsisWrapper->cmsisMutex) != KOSA_StatusSuccess))
    {
        return ARM_DRIVER_ERROR;
    }

    return status;
}
