// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * Author: Jianchao Wang
 */
#include <sfw.h>
#include <stdint.h>
#include <string.h>
#include "flash_map.h"
#include "sysflash.h"

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

//extern flash_ops_s mcuboot_flash;

static struct flash_area flash_map[IMAGE_SLOT_NUM] = 
{
	FLASH_AREA_INIT(1, 1)
	FLASH_AREA_INIT(2, 1)
	FLASH_AREA_INIT(3, 1)
};

int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
	if (fd_id == FLASH_DEVICE_ID)
		*ret = FLASH_DEVICE_BASE_ADDR;
	else
		return -1;
	
	return 0;
}

int flash_area_open(uint8_t id, const struct flash_area **area)
{
	uint32_t i = 0;
	
	for (i = 0; i < IMAGE_SLOT_NUM; i++) {
		if (flash_map[i].fa_id == id) {
			*area = &flash_map[i];
			return 0;
		}
	}
	
	return -1;
}

void flash_area_close(const struct flash_area *area)
{

}
#if 0
/*
 * Read/write/erase. Offset is relative from beginning of flash area.
 */
int flash_area_read(const struct flash_area *area, uint32_t off, void *dst, uint32_t len)
{
	if (area->fa_device_id == FLASH_DEVICE_ID) {
		mcuboot_flash.flash_read(area->fa_off + off, dst, len);
	}
	return 0;
}

/* Make sure the data address is 4 bytes aligned */
int flash_area_write(const struct flash_area *area, uint32_t off, const void *src, uint32_t len)
{
	if (area->fa_device_id == FLASH_DEVICE_ID) {
		return mcuboot_flash.flash_write(area->fa_off + off, src, len);
	}
	
	return -1;
}

int flash_area_erase(const struct flash_area *area, uint32_t off, uint32_t len)
{
	if (area->fa_device_id == FLASH_DEVICE_ID) {
		return mcuboot_flash.flash_erase(area->fa_off + off, len);
	}
	return -1;
}

/* The minimum write size */
uint8_t flash_area_align(const struct flash_area *area)
{
	if (area->fa_device_id == FLASH_DEVICE_ID)
		return mcuboot_flash.align_val;
	else
		return 0;
}

#define ERASED_VAL 0xFF
uint8_t flash_area_erased_val(const struct flash_area *area)
{
	return mcuboot_flash.erased_val;
}

int flash_area_read_is_empty(const struct flash_area *area, uint32_t off, void *dst, uint32_t len)
{
	uint8_t i;
    uint8_t *u8dst;
    int rc;

    rc = flash_area_read(area, off, dst, len);
    if (rc) {
        return -1;
    }

    for (i = 0, u8dst = (uint8_t *)dst; i < len; i++) {
        if (u8dst[i] != mcuboot_flash.erased_val) {
            return 0;
        }
    }

    return 1;
}
#endif
/*
 * Lookup the sector map for a given flash area.  This should fill in
 * `sectors` with all of the sectors in the area.  `*count` will be set to
 * the storage at `sectors` and should be set to the final number of
 * sectors in this area.
 */
int flash_area_get_sectors(int fa_id, uint32_t *count, struct flash_sector *sectors)
{
	const struct flash_area *fa;
	uint32_t max_cnt = *count;
    uint32_t rem_len;
	int rc = -1;
	
	if (flash_area_open(fa_id, &fa))
		goto out;
	
	if (*count < 1)
		goto fa_close_out;
	
	rem_len = fa->fa_size;
    *count = 0;
	while ((rem_len > 0) && (*count < max_cnt)) {
		if (rem_len < FLASH_AREA_IMAGE_SECTOR_SIZE) {
			goto fa_close_out;
		}
		
		sectors[*count].fs_off = FLASH_AREA_IMAGE_SECTOR_SIZE * (*count);
        sectors[*count].fs_size = FLASH_AREA_IMAGE_SECTOR_SIZE;
        *count = *count + 1;
        rem_len -= FLASH_AREA_IMAGE_SECTOR_SIZE;
	}
	
	if (*count >= max_cnt) {
		goto fa_close_out;
	}
	
	rc = 0;

fa_close_out:
    flash_area_close(fa);
out:
    return rc;
}

/*
 * This depends on the mappings defined in sysflash.h.
 * MCUBoot uses continuous numbering for the primary slot, the secondary slot,
 * and the scratch while zephyr might number it differently.
 */
int flash_area_id_from_image_slot(int slot)
{
	static const int area_id_tab[] = {FLASH_AREA_IMAGE_PRIMARY,
                                      FLASH_AREA_IMAGE_SECONDARY,
                                      FLASH_AREA_IMAGE_SCRATCH};

    if (slot >= 0 && slot < ARRAY_SIZE(area_id_tab)) {
        return area_id_tab[slot];
    }

    return -1; /* flash_area_open will fail on that */
}

int flash_area_id_to_image_slot(int area_id)
{
	return (area_id - 1);
}
