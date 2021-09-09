/*
* Copyright 2021 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef SFW_CONFIG_H__
#define SFW_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* MCU-SFW RT1060 Configuration */

#define SOC_IMXRT1060_SERIES
#define ARCH_ARM_CORTEX_M7
#define ARCH_ARM_CORTEX_FPU
#define SOC_IMXRTYYYY_SERIES
#define SOC_REMAP_ENABLE

/* MCU SFW core */

#define SFW_RTOS_FREERTOS
#define OTA_SUPPORT
#define OTA_CLOUD
#define OTA_ALIYUN

/* Aliyun Config */

#define PRODUCT_KEY ""
#define DEVICE_NAME ""
#define DEVICE_SECRET ""
#define OTA_SDCARD
#define COMPONENT_SDMMC
#define OTA_UDISK
#define COMPONENT_FS
#define COMPONENT_USB
#define COMPONENT_OSA

/* MCU SFW Flash Map */

#define BOOT_FLASH_BASE 0x60000000
#define BOOT_FLASH_HEADER 0x60010000
#define BOOT_FLASH_ACT_APP 0x60100000
#define BOOT_FLASH_CAND_APP 0x60200000
#define BOOT_FLASH_CUSTOMER 0x603f0000

/* MCU SFW metadata header */


/* MCU SFW Component */

/* Flash IAP */

#define COMPONENT_FLASHIAP
#define COMPONENT_FLASHIAP_ROM

/* Flash device parameters */

#define COMPONENT_FLASHIAP_SIZE 8388608

/* secure */

#define COMPONENT_MBEDTLS
#define SFW_MBEDTLS_CONFIG_FILE "ksdk_mbedtls_config.h"

/* Serial Manager */

#define COMPONENT_SERIAL_MANAGER
#define COMPONENT_SERIAL_MANAGER_LPUART
#define SERIAL_PORT_TYPE_UART 1

/* Platform Drivers Config */

#define BOARD_FLASH_SUPPORT
#define ISSI_IS25WPxxxA
#define SOC_MIMXRT1062DVL6A

/* On-chip Peripheral Drivers */

#define SOC_GPIO
#define SOC_LPUART
#define SOC_LPUART_1
#define SOC_FLEXSPI
#define SOC_FLEXSPI_1

/* Onboard Peripheral Drivers */

#define BOARD_ENET
#define COMPONENT_PHY

/* Board extended module Drivers */


#endif
