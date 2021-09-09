/*
* Copyright 2021 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef SFW_CONFIG_H__
#define SFW_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* MCU-SFW RT600 Configuration */

#define SOC_IMXRT600_SERIES
#define ARCH_ARM_CORTEX_M33
#define ARCH_ARM_CORTEX_FPU
#define SOC_IMXRTXXX_SERIES
#define SOC_REMAP_ENABLE

/* MCU SFW core */

#define SFW_RTOS_FREERTOS
#define OTA_SUPPORT
#define OTA_SDCARD
#define COMPONENT_SDMMC
#define OTA_UDISK
#define COMPONENT_FS
#define COMPONENT_USB
#define COMPONENT_OSA

/* MCU SFW Flash Map */

#define BOOT_FLASH_BASE 0x08000000
#define BOOT_FLASH_HEADER 0x08010000
#define BOOT_FLASH_ACT_APP 0x08100000
#define BOOT_FLASH_CAND_APP 0x08200000
#define BOOT_FLASH_CUSTOMER 0x083f0000

/* MCU SFW metadata header */


/* MCU SFW Component */

/* Flash IAP */

#define COMPONENT_FLASHIAP
#define COMPONENT_FLASHIAP_ROM

/* Flash device parameters */

#define COMPONENT_FLASHIAP_SIZE 67108864

/* secure */


/* Serial Manager */

#define COMPONENT_SERIAL_MANAGER
#define COMPONENT_SERIAL_MANAGER_LPUART
#define SERIAL_PORT_TYPE_UART 1

/* Platform Drivers Config */

#define BOARD_FLASH_SUPPORT
#define Macronix_MX25UM51345G_2nd
#define SOC_MIMXRT685S_M33

/* On-chip Peripheral Drivers */

#define SOC_GPIO
#define SOC_UART
#define SOC_FLEXSPI
#define SOC_FLEXSPI_0
#define SOC_FLEXSPI_1
#define SOC_INPUTMUX
#define SOC_PINT

/* Onboard Peripheral Drivers */


/* Board extended module Drivers */


#endif
