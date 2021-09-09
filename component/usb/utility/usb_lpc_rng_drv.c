/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_rng.h"

void USB_RandInit(uint8_t instance, uint32_t seed)
{
}

void USB_RandDeinit(uint8_t instance)
{
    return;
}

uint8_t USB_RandGet(uint8_t instance)
{
    return RNG_GetRandomData();
}

uint8_t USB_RandGetMulti(uint8_t instance, uint8_t *data, uint32_t count)
{
    uint32_t index = 0;
    for (index = 0; index < count; ++index)
    {
        data[index] = RNG_GetRandomData();
    }
    return 1;
}
