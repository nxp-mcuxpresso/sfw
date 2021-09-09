/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexspi.h"
#include "mflash_drv.h"
#include "pin_mux.h"
#include <stdbool.h>
#include "flexspi_flash.h"
#include "fsl_debug_console.h"
#include "flexspi_flash_config.h"

/* Internal - erase single sector */
static int32_t mflash_drv_sector_erase_internal(uint32_t sector_addr)
{
    uint32_t primask = __get_PRIMASK();

    __asm("cpsid i");

    status_t status;

    status = flexspi_nor_flash_erase_sector(MFLASH_FLEXSPI, sector_addr);

    if (primask == 0)
    {
        __asm("cpsie i");
    }
    /* Flush pipeline to allow pending interrupts take place
     * before starting next loop */
    __ISB();

    if (status != kStatus_Success)
    {
        return -1;
    }

    return 0;
}

/* Calling wrapper for 'mflash_drv_sector_erase_internal'.
 * Erase one sector starting at 'sector_addr' - must be sector aligned.
 */
int32_t mflash_drv_sector_erase(uint32_t sector_addr)
{
    if (false == mflash_drv_is_sector_aligned(sector_addr))
        return -1;

    volatile int32_t result;
    result = mflash_drv_sector_erase_internal(sector_addr);
    return result;
}

/* Internal - write single page */
static int32_t mflash_drv_page_program_internal(uint32_t page_addr, uint32_t *data)
{
    uint32_t primask = __get_PRIMASK();

    __asm("cpsid i");

    status_t status;

    status = flexspi_nor_flash_program(MFLASH_FLEXSPI, page_addr, data, MFLASH_PAGE_SIZE);

    if (primask == 0)
    {
        __asm("cpsie i");
    }
    /* Flush pipeline to allow pending interrupts take place
     * before starting next loop */
    __ISB();

    if (status != kStatus_Success)
    {
        return -1;
    }

    return 0;
}

/* Calling wrapper for 'mflash_drv_page_program_internal'.
 * Write 'data' to 'page_addr' - must be page aligned.
 * NOTE: Don't try to store constant data that are located in XIP !!
 */
int32_t mflash_drv_page_program(uint32_t page_addr, uint32_t *data)
{
    if (false == mflash_drv_is_page_aligned(page_addr))
        return -1;

    volatile int32_t result;
    result = mflash_drv_page_program_internal(page_addr, data);
    return result;
}

/* Internal - read data */
static int32_t mflash_drv_read_internal(uint32_t addr, uint32_t *buffer, uint32_t length)
{
    uint32_t primask = __get_PRIMASK();

    __asm("cpsid i");

    status_t status;

    status = sfw_flash_read_ipc(addr, buffer, length);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(MFLASH_FLEXSPI);

    if (primask == 0)
    {
        __asm("cpsie i");
    }
    /* Flush pipeline to allow pending interrupts take place
     * before starting next loop */
    __ISB();

    if (status != kStatus_Success)
    {
        return -1;
    }

    return 0;
}

/* Calling wrapper for 'mflash_drv_read_internal'. */
int32_t mflash_drv_read(uint32_t addr, uint32_t *buffer, uint32_t length)
{
    /* Check alignment */
    if (((uint32_t)buffer % 4) || (length % 4))
        return -1;

    volatile int32_t result;
    result = mflash_drv_read_internal(addr, buffer, length);
    return result;
}

/* API - initialize 'mflash' */
int32_t mflash_drv_init(void)
{
    volatile int32_t result;
    /* Necessary to have double wrapper call in non_xip memory */
    //result = mflash_drv_init_internal();
    result = 0;

    return result;
}

/* Temporary sector shadow buffer. Use uint32_t type to force 4B alignment and
 * improve copy operation */
static uint32_t g_flashm_sector[MFLASH_SECTOR_SIZE / sizeof(uint32_t)];

/* Internal - write data of 'data_len' to single sector 'sector_addr', starting from 'sect_off' */
static int32_t mflash_drv_sector_update(uint32_t sector_addr, uint32_t sect_off, const uint8_t *data, uint32_t data_len)
{
    int sector_erase_req      = 0;
    uint32_t page_program_map = 0; /* Current implementation is limited to 32 pages per sector */

    /* Address not aligned to sector boundary */
    if (false == mflash_drv_is_sector_aligned(sector_addr))
        return -1;
    /* Offset + length exceeed sector size */
    if (sect_off + data_len > MFLASH_SECTOR_SIZE)
        return -1;

    if (0 != mflash_drv_read(sector_addr, &g_flashm_sector[0], sizeof(g_flashm_sector)))
    {
        return -2;
    }

    /* Diff the data to determine pages to be programed */
    for (uint32_t i = 0; i < data_len; i++)
    {
        uint8_t cur_value = ((uint8_t *)(g_flashm_sector))[sect_off + i];
        uint8_t new_value = data[i];

        if ((cur_value | new_value) != cur_value)
        {
            /* A bit needs to be flipped from 0 to 1, the whole sector has to be erased */
            sector_erase_req = 1;
            break;
        }

        if (cur_value != new_value)
        {
            /* There is a change, the page has to be programmed for sure */
            page_program_map |= 1 << ((sect_off + i) / MFLASH_PAGE_SIZE);
        }
    }

#if !defined(MFLASH_INC_WRITES) || !MFLASH_INC_WRITES
    /* Perform blank check page by page until decission for sector erase is made or we reach last page of the sector */
    for (int page_idx = 0; (0 == sector_erase_req) && page_idx < MFLASH_SECTOR_SIZE / MFLASH_PAGE_SIZE; page_idx++)
    {
        /* Check only pages which need to be programed */
        if (page_program_map & (1 << page_idx))
        {
            int page_word_start = page_idx * (MFLASH_PAGE_SIZE / sizeof(g_flashm_sector[0]));
            int page_word_end   = page_word_start + (MFLASH_PAGE_SIZE / sizeof(g_flashm_sector[0]));

            for (int i = page_word_start; i < page_word_end; i++)
            {
                if (g_flashm_sector[i] != 0xFFFFFFFF)
                {
                    /* Mark sector to be erased and quit */
                    sector_erase_req = 1;
                    break;
                }
            }
        }
    }
#endif

    /* Copy data to be programmed byte by byte to shadow buffer at proper position */
    for (uint32_t i = 0; i < data_len; i++)
    {
        ((uint8_t *)g_flashm_sector)[sect_off + i] = data[i];
    }

    /* If sector is to be erased, update page program map according to non-blank areas in the shadow buffer */
    if (0 != sector_erase_req)
    {
        for (int page_idx = 0; page_idx < MFLASH_SECTOR_SIZE / MFLASH_PAGE_SIZE; page_idx++)
        {
            int page_word_start = page_idx * (MFLASH_PAGE_SIZE / sizeof(g_flashm_sector[0]));
            int page_word_end   = page_word_start + (MFLASH_PAGE_SIZE / sizeof(g_flashm_sector[0]));

            for (int i = page_word_start; i < page_word_end; i++)
            {
                if (g_flashm_sector[i] != 0xFFFFFFFF)
                {
                    /* Mark the page for programming and go for next one */
                    page_program_map |= (1 << page_idx);
                    break;
                }
            }
        }
    }

    /* Erase the sector if required */
    if (0 != sector_erase_req)
    {
        if (0 != mflash_drv_sector_erase(sector_addr))
        {
            return -2;
        }
    }

    /* Program the required pages */
    for (int page_idx = 0; page_idx < MFLASH_SECTOR_SIZE / MFLASH_PAGE_SIZE; page_idx++)
    {
        if (page_program_map & (1 << page_idx))
        {
            if (0 !=
                mflash_drv_page_program(sector_addr + page_idx * MFLASH_PAGE_SIZE,
                                        g_flashm_sector + page_idx * (MFLASH_PAGE_SIZE / sizeof(g_flashm_sector[0]))))
            {
                return -3;
            }
        }
    }

    // mflash_drv_read_mode();
    return 0;
}

/* Write data to flash, cannot be invoked directly, requires calling wrapper in non XIP memory */
int32_t mflash_drv_write_internal(uint32_t addr, const uint8_t *data, uint32_t data_len)
{
    /* Interval <0, sector_size) */
    uint32_t to_write = 0;
    /* Interval (data_len, 0>  */
    uint32_t to_remain = data_len;
    /* Physical address in external FLASH device */

    int32_t result = 0;

    for (
        /* Calculate address of first sector */
        uint32_t sect_a = (addr / MFLASH_SECTOR_SIZE) * MFLASH_SECTOR_SIZE,
                 /* and first sector offset */
        sect_of = addr % MFLASH_SECTOR_SIZE,
                 /* and set first data offset to 0*/
        data_of = 0;
        /* Continue until sector address exceed target adddress + data_length */
        sect_a < addr + data_len;
        /* Move to next sector */
        sect_a += MFLASH_SECTOR_SIZE,
                 /* and move pointer to data */
        data_of += to_write)
    {
        /* If remaining data is exceed 'sector_size', write 'sector_size' length */
        if (to_remain > MFLASH_SECTOR_SIZE - sect_of)
        {
            to_write  = MFLASH_SECTOR_SIZE - sect_of;
            to_remain = to_remain - to_write;
        }
        /* else write remaining data length */
        else
        {
            to_write  = to_remain;
            to_remain = 0;
        }

        /* Write at 'sect_a' sector, starting at 'sect_of' using '&data[data_of]' of length 'to_write' */
        result = mflash_drv_sector_update(sect_a, sect_of, data + data_of, to_write);
        if (0 != result)
            return -1;
        /* Only first sector is allowed to have an offset */
        sect_of = 0;
    }

    return 0;
}

/* Calling wrapper for 'mflash_drv_write_internal'.
 * Write 'data' of 'data_len' to 'any_addr' - which doesn't have to be sector aligned.
 * NOTE: Don't try to store constant data that are located in XIP !!
 */
int32_t mflash_drv_write(uint32_t addr, const uint8_t *data, uint32_t data_len)
{
    volatile int32_t result;
    result = mflash_drv_write_internal(addr, data, data_len);
    return result;
}
