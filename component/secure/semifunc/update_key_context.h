// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 */
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "sfw.h"
#include "fsl_debug_console.h"
#include "flash_info.h"

#ifndef _UPDATE_KEY_CONTEXT_H_
#define _UPDATE_KEY_CONTEXT_H_

#ifdef SOC_REMAP_ENABLE
#   undef REMAPADDRSTART
#   if defined(SOC_IMXRT1170_SERIES)
#   define REMAPADDRSTART  (0x400CC420)
#   elif defined(SOC_IMXRT1060_SERIES) || defined(SOC_IMXRT1064_SERIES)
#   define REMAPADDRSTART  (0x400AC078)
#   elif defined(SOC_IMXRT1010_SERIES)
#   define REMAPADDRSTART  (0x400AC06C)
#   elif defined(SOC_IMXRT600_SERIES)
#   define REMAPADDRSTART   (FLEXSPI_BASE + 0x420)
#   elif defined(SOC_IMXRT500_SERIES)
#   define REMAPADDRSTART   (FLEXSPI0_BASE + 0x420)
#   else
#   error "Doesn't define REMAPADDRSTART"
#   endif
#endif

/* Define the key context offset in user application image
 */
#define KEY_CONTEXT_OFFSET_IN_APP       (0x100u)

/* Assume that user encrypt SBL, copy new key inormation to second context,
 * or user should change the offset of key context
 */
#if defined(SOC_IMXRT1170_SERIES) || defined(SOC_IMXRT1010_SERIES)  || \
    defined(SOC_IMXRTXXX_SERIES)
#define KEY_CONTEXT2_OFFSET_IN_BOOTIMAGE    (0x40u)
#define KEY_CONTEXT_SIZE                    (0x40u)
#elif defined(SOC_IMXRT1060_SERIES) || defined(SOC_IMXRT1050_SERIES) || \
      defined(SOC_IMXRT1064_SERIES) || defined(SOC_IMXRT1020_SERIES)
#define KEY_CONTEXT2_OFFSET_IN_BOOTIMAGE    (0x800u)
#define KEY_CONTEXT_SIZE                    (0x180u)
#endif

/*!
 * @brief Update application key context information.
 *
 * @ret Update key context success or not.
 * Note: User should update application key context after write the image trailer.
 */
bool update_key_context(void);

#endif // _UPDATE_KEY_CONTEXT_H_

