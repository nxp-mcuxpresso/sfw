/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_rnga.h"

void USB_RandInit(uint8_t instance, uint32_t seed)
{
    RNG_Type *rngArr[] = RNG_BASE_PTRS;

    /* Init RNGA */
    RNGA_Init(rngArr[instance]);
}

void USB_RandDeinit(uint8_t instance)
{
    RNG_Type *rngArr[] = RNG_BASE_PTRS;

    RNGA_Deinit(rngArr[instance]);
    return;
}

uint8_t USB_RandGet(uint8_t instance)
{
    RNG_Type *rngArr[] = RNG_BASE_PTRS;
    uint8_t data;

    RNGA_GetRandomData(rngArr[instance], &data, 1);
    return data;
}

uint8_t USB_RandGetMulti(uint8_t instance, uint8_t *data, uint32_t count)
{
    RNG_Type *rngArr[] = RNG_BASE_PTRS;
    RNGA_GetRandomData(rngArr[instance], data, count);
    return 1;
}
