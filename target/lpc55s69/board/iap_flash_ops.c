/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "iap_flash_ops.h"

static flash_config_t flashInstance;
static uint8_t is_flash_init = 0;

/*! @brief Alignment(down) utility. */
#if !defined(ALIGN_DOWN)
#define ALIGN_DOWN(x, a) ((x) & (uint32_t)(-((int32_t)(a))))
#endif

/*! @brief Alignment(up) utility. */
#if !defined(ALIGN_UP)
#define ALIGN_UP(x, a) (-((int32_t)((uint32_t)(-((int32_t)(x))) & (uint32_t)(-((int32_t)(a))))))
#endif

#include "fsl_debug_console.h"
static uint8_t  write_cache[FLASH_PAGE_SIZE*2];

status_t sfw_flash_erase(uint32_t address, size_t len)
{
    //PRINTF("I AM V1.2!!!!! %s 0x%X 0x%X\r\n", __FUNCTION__, address, len);
    return FLASH_Erase(&flashInstance, address, ALIGN_UP(len, FLASH_PAGE_SIZE), kFLASH_ApiEraseKey);
}

status_t sfw_flash_write(uint32_t dstAddr, const void *src, size_t len)
{
    uint32_t pz_ali_addr = ALIGN_DOWN(dstAddr, FLASH_PAGE_SIZE);
    uint32_t offset_in_sec = dstAddr - pz_ali_addr;
    int ret = 0;
    
    //PRINTF("I AM V1.2!!! %s addr:0x%X len:0x%X offset_in_sec:%d\r\n", __FUNCTION__, dstAddr, len, offset_in_sec);
    
//    
//    sfw_flash_read(pz_ali_addr, write_cache, sizeof(write_cache));
//    sfw_flash_erase(pz_ali_addr, sizeof(write_cache));
//    memcpy(&write_cache[offset_in_sec], src, len);
//    FLASH_Program(&flashInstance, pz_ali_addr, write_cache, sizeof(write_cache));
//    
    if(offset_in_sec == 0 && (len % FLASH_PAGE_SIZE == 0))
    {
        return FLASH_Program(&flashInstance, dstAddr, (uint8_t*)src, len);
    }
    else
    {
        int i;
        for(i=0; i<(sizeof(write_cache) / FLASH_PAGE_SIZE); i++)
        {
            ret += sfw_flash_read(pz_ali_addr+FLASH_PAGE_SIZE*i, write_cache+FLASH_PAGE_SIZE*i, FLASH_PAGE_SIZE);
        }    
        
        ret = sfw_flash_read(pz_ali_addr, write_cache, sizeof(write_cache));
        sfw_flash_erase(pz_ali_addr, sizeof(write_cache));
        memcpy(&write_cache[offset_in_sec], src, len);
        return FLASH_Program(&flashInstance, pz_ali_addr, write_cache, sizeof(write_cache));
    }
}

status_t sfw_flash_read(uint32_t dstAddr, void *buf, size_t len)
{
    int ret;
    
    if(!is_flash_init)
    {
        sfw_flash_init();
    }

    ret = FLASH_Read(&flashInstance, dstAddr, buf, len);
    return ret;
}


status_t sfw_flash_init(void)
{
    is_flash_init = 1;
    return FLASH_Init(&flashInstance);
}
