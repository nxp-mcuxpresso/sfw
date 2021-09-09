/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FLEXSPI_FLASH_H_
#define _FLEXSPI_FLASH_H_

/*${header:start}*/
#include "fsl_cache.h"
/*${header:end}*/
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_FLEXSPI                 FLEXSPI
#define FLASH_SIZE_KB                   (COMPONENT_FLASHIAP_SIZE/1024) /* 0x2000= 64Mb/KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI_AMBA_BASE
#define FLASH_PAGE_SIZE                 FLASH_CONFIG_PAGESIZE
#define SECTOR_SIZE                     (FLASH_CONFIG_SECTORSIZE) /* 4K */
#define EXAMPLE_FLEXSPI_CLOCK           kCLOCK_Flexspi
#define FLASH_PORT                      kFLEXSPI_PortB1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackInternally
#define FLEXSPI_BASE_ADDRESS_MASK (FLASH_SIZE_KB * 0x400 -1)

#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0
#define FLASH_ERROR_STATUS_MASK  0x0e
#define FLASH_ENABLE_OCTAL_CMD   0x02

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/

status_t sfw_flash_erase(uint32_t address, size_t len);
status_t sfw_flash_write(uint32_t dstAddr, const void *src, size_t len);
status_t sfw_flash_read(uint32_t dstAddr, void *buf, size_t len);
status_t sfw_flash_init(void);
status_t sfw_flash_read_ipc(uint32_t address, void *buffer, size_t length);

/*${prototype:end}*/

#endif /* _FLEXSPI_FLASH_H_ */
