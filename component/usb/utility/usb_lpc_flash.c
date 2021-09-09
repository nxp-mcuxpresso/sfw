/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_flashiap.h"
#include "usb_flash.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief flash configuration */
uint32_t startSector;
uint32_t endSector;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief USB memmory erasing function.
 *
 * This function erases the memmory area from start address to the end.
 *
 * @param address  The start address.
 * @param address  The erase size.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryCalcuateSectorNum(uint32_t address, uint32_t size)
{
    usb_memmory_status_t flashstatus = kStatus_USB_MemmorySuccess;
    if (!size)
    {
        return kStatus_USB_MemmoryErrorUnknown;
    }
    startSector = address / (FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES);
    if ((address + size) / (FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES) >= (startSector))
    {
        endSector = (address + size) / (FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES);
    }
    else
    {
        endSector = startSector;
    }

    return flashstatus;
}
/*!
 * @brief USB memmory initialization function.
 *
 * This function initializes the memmory driver structure and variables.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryInit(void)
{
    usb_memmory_status_t status = kStatus_USB_MemmorySuccess;
    return status;
}
/*!
 * @brief USB memmory erasing function.
 *
 * This function erases the memmory area from start address to the end.
 *
 * @param address  The start address.
 * @param address  The erase size.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryErase(uint32_t address, uint32_t size)
{
    usb_memmory_status_t flashstatus = kStatus_USB_MemmorySuccess;
    uint32_t status;
    USB_MemmoryCalcuateSectorNum(address, size);
    FLASHIAP_PrepareSectorForWrite(startSector, endSector);
    status = FLASHIAP_EraseSector(startSector, endSector, SystemCoreClock);

    if (kStatus_FLASHIAP_Success != status)
    {
        flashstatus = kStatus_USB_MemmoryErrorErase;
        return flashstatus;
    }
    return flashstatus;
}

/*!
 * @brief USB memmory programming function.
 *
 * This function program memmory with data at locations passed in through parameters.
 *
 * @param address The start address to be programmed.
 * @param buffer  Pointer to buffer data.
 * @param length  The length of data in byte.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryProgram(uint32_t address, uint8_t *buffer, uint32_t length)
{
    usb_memmory_status_t flashstatus = kStatus_USB_MemmorySuccess;
    uint32_t status;
    if (length % (FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES))
    {
        length = (length / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES + 1) * (FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES);
    }

    FLASHIAP_PrepareSectorForWrite(startSector, endSector);
    {
        FLASHIAP_CopyRamToFlash(address, (uint32_t *)&buffer[0], length, SystemCoreClock);
    }
    status = FLASHIAP_Compare(address, (uint32_t *)&buffer[0], length);
    if (status != kStatus_FLASHIAP_Success)
    {
        flashstatus = kStatus_USB_MemmoryErrorProgramVerify;
    }
    return flashstatus;
}
