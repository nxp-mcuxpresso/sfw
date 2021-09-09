// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 */

#include "update_key_context.h"

#if !defined(SINGLE_IMAGE) && defined(CONFIG_BOOT_ENCRYPTED_XIP)

static uint8_t s_buffer[SECTOR_SIZE];
static status_t do_update_key_context(uint32_t key_info_offset);

#if defined(SOC_REMAP_ENABLE)
bool is_enable_remap(void)
{
    return (*(volatile uint32_t *)REMAPADDRSTART != 0);
}
#endif

#if defined(SOC_REMAP_ENABLE)
bool update_key_context(void)
{
    status_t status = kStatus_Fail;

    if (is_enable_remap())
    {
        // applicaton is in slot2, copy key context from slot1
        status = do_update_key_context(FLASH_AREA_IMAGE_1_OFFSET + KEY_CONTEXT_OFFSET_IN_APP);
    }
    else
    {
        // applicaton is in slot1, copy key context from slot2
        status = do_update_key_context(FLASH_AREA_IMAGE_2_OFFSET + KEY_CONTEXT_OFFSET_IN_APP);
    }

    return (status == kStatus_Success);
}
#else
bool update_key_context(void)
{
    return (kStatus_Success == do_update_key_context(FLASH_AREA_IMAGE_2_OFFSET + KEY_CONTEXT_OFFSET_IN_APP));
}
#endif // defined(SOC_REMAP_ENABLE)

static status_t do_update_key_context(uint32_t key_info_offset)
{
    status_t status = kStatus_Fail;
    uint32_t primask;
    uint32_t key_info[KEY_CONTEXT_SIZE/4];

    do {
        status = sfw_flash_read(0, s_buffer, SECTOR_SIZE);
        if (status != kStatus_Success) {
            break;
        }

        status = sfw_flash_read_ipc(key_info_offset, (uint8_t *)key_info, KEY_CONTEXT_SIZE);
        if (status != kStatus_Success) {
            PRINTF(" Faield to read key information\r\n");
            break;
        }

        /* Assume that user encrypt SBL, copy new key inormation to second context,
         * or user should use the real destination address
         */
        memcpy(s_buffer + KEY_CONTEXT2_OFFSET_IN_BOOTIMAGE, key_info, KEY_CONTEXT_SIZE);

        primask = DisableGlobalIRQ();
        status = sfw_flash_erase(0, SECTOR_SIZE);
        EnableGlobalIRQ(primask);
        if (status != kStatus_Success) {
            PRINTF(" Faield to erase\r\n");
            break;
        }

        primask = DisableGlobalIRQ();
        status = sfw_flash_write(0, s_buffer, SECTOR_SIZE);
        EnableGlobalIRQ(primask);
        if (status != kStatus_Success) {
            break;
        }

    } while (0);
    
    return status;
}
#endif //!defined(SINGLE_IMAGE) && defined(CONFIG_BOOT_ENCRYPTED_XIP)
