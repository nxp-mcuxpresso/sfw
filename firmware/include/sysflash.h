// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 */
#ifndef __SYSFLASH_H__
#define __SYSFLASH_H__

#include <string.h>
#include "flash_info.h"

#define FLASH_AREA_IMAGE_PRIMARY    1
#define FLASH_AREA_IMAGE_SECONDARY  2
#define FLASH_AREA_IMAGE_SCRATCH    3

#define IMAGE_SLOT_NUM				3

#define FLASH_DEVICE_ID				1

#define FLASH_AREA_INIT(i, j)						\
        {.fa_id = i,								\
		 .fa_device_id = j,							\
         .fa_off = FLASH_AREA_IMAGE_##i##_OFFSET,	\
         .fa_size = FLASH_AREA_IMAGE_##i##_SIZE,},

typedef int (*init_t)(void);
typedef int (*erase_t)(uint32_t, size_t);
typedef int (*read_t)(uint32_t, void*, size_t);
typedef int (*write_t)(uint32_t, const void*, size_t);

typedef struct {
	init_t flash_init;
	erase_t flash_erase;
	read_t flash_read;
	write_t flash_write;
	uint8_t align_val;
	uint8_t erased_val;
} flash_ops_s;

#endif /* __SYSFLASH_H__ */
