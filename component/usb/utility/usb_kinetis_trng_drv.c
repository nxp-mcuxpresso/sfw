/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_trng.h"

void USB_RandInit(uint8_t instance, uint32_t seed)
{
    TRNG_Type *trngArr[] = TRNG_BASE_PTRS;

    trng_config_t trngConfig;
    TRNG_GetDefaultConfig(&trngConfig);
    /* Set sample mode of the TRNG ring oscillator to Von Neumann, for better random data.
     * It is optional.*/
    trngConfig.sampleMode = kTRNG_SampleModeVonNeumann;
    /* Initialize TRNG */
    TRNG_Init(trngArr[instance], &trngConfig);
}

void USB_RandDeinit(uint8_t instance)
{
    TRNG_Type *trngArr[] = TRNG_BASE_PTRS;

    TRNG_Deinit(trngArr[instance]);
    return;
}

uint8_t USB_RandGet(uint8_t instance)
{
    TRNG_Type *trngArr[] = TRNG_BASE_PTRS;
    uint8_t data;

    TRNG_GetRandomData(trngArr[instance], &data, 1);
    return data;
}

uint8_t USB_RandGetMulti(uint8_t instance, uint8_t *data, uint32_t count)
{
    TRNG_Type *trngArr[] = TRNG_BASE_PTRS;
    TRNG_GetRandomData(trngArr[instance], data, count);
    return 1;
}
