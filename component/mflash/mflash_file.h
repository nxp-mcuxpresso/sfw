/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MFLASH_FILE__
#define __MFLASH_FILE__

#include "mflash_drv.h"
#include <sfw.h>

typedef struct
{
    uint32_t flash_addr;
    uint32_t max_size;
    char path[64];
} mflash_file_t;

#ifdef SOC_IMXRT1064_SERIES
#define MFLASH_FILE_BASEADDR (0x00380000)
#elif defined(SOC_IMXRT1060_SERIES) || defined(SOC_IMXRT1020_SERIES)
#define MFLASH_FILE_BASEADDR (0x00580000)
#elif defined(SOC_IMXRT1170_SERIES)
#define MFLASH_FILE_BASEADDR (0x00D90000)
#elif defined(SOC_IMXRT1050_SERIES)
#define MFLASH_FILE_BASEADDR (0x03D80000)
#endif
#define MFLASH_FILE_SIZE MFLASH_SECTOR_SIZE

bool mflash_is_initialized(void);

BaseType_t mflash_init(mflash_file_t *user_file_table, bool init_drv);

BaseType_t mflash_read_file(char *pcFileName, uint8_t **ppucData, uint32_t *pulDataSize);

BaseType_t mflash_save_file(char *pcFileName, uint8_t *pucData, uint32_t ulDataSize);

#endif
