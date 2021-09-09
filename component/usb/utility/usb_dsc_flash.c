/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_flash.h"
#include "rom_flash_api.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
flash_config_t dfuFlashConfig[] = {
    // Main Flash
    {0}};
uint32_t pflashBlockBase  = 0;
uint32_t pflashTotalSize  = 0;
uint32_t pflashSectorSize = 0;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

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

    status = FLASH_Init(dfuFlashConfig);
    if (kStatus_FLASH_Success == status)
    {
        return kStatus_USB_MemmorySuccess;
    }
    else
    {
        return status;
    }
}
/*!
 * @brief USB memmory erasing function.
 *
 * This function erases the memmory area from start address to the end.
 *
 * @param address  The start address, must be phrase-aligned (64 bits).
 * @param address  The erase size, must be phrase-aligned (64 bits).
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryErase(uint32_t address, uint32_t size)
{
    usb_memmory_status_t status = kStatus_USB_MemmorySuccess;
    uint32_t eraseLength;

    eraseLength = (size / 8U + 1) * 8U; /* make the length is align 64 bits */
    if (kStatus_FLASH_Success != FLASH_Erase(dfuFlashConfig, address, eraseLength, kFLASH_ApiEraseKey))
    {
        status = kStatus_USB_MemmoryErrorErase;
        return status;
    }
    if (kStatus_FLASH_Success != FLASH_VerifyErase(dfuFlashConfig, address, eraseLength, kFLASH_MarginValueUser))
    {
        status = kStatus_USB_MemmoryErrorEraseVerify;
    }
    return status;
}

/*!
 * @brief USB memmory programming function.
 *
 * This function program memmory with data at locations passed in through parameters.
 *
 * @param address The start address to be programmed.
 * @param buffer  Pointer to buffer data, but must be phrase-aligned (64 bits).
 * @param length  The length of data in byte, but must be phrase-aligned (64 bits).
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryProgram(uint32_t address, uint8_t *buffer, uint32_t length)
{
    usb_memmory_status_t status = kStatus_USB_MemmorySuccess;
    status_t flash_status       = kStatus_FLASH_Success;
    uint32_t programLength;

    if (length % 8U)
    {
        programLength = (length / 8U + 1) * 8U;
    }
    flash_status = FLASH_ProgramSection(dfuFlashConfig, address, (uint8_t *)buffer, programLength);
    if (kStatus_FLASH_Success != flash_status)
    {
        status = kStatus_USB_MemmoryErrorProgram;
        return status;
    }
    flash_status =
        FLASH_VerifyProgram(dfuFlashConfig, address, length, (uint8_t *)buffer, kFLASH_MarginValueUser, NULL, NULL);

    if (kStatus_FLASH_Success != flash_status)
    {
        status = kStatus_USB_MemmoryErrorProgramVerify;
    }
    return status;
}
