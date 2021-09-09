/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#ifdef COMPONENT_MBEDTLS
#include "ksdk_mbedtls.h"
#endif
#include "fsl_iomuxc.h"

#include <sfw.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
void BOARD_InitModuleClock(void)
{
    const clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};
    CLOCK_InitEnetPll(&config);
}

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    SCB_DisableDCache();
	
#ifdef OTA_CLOUD
#ifdef COMPONENT_PHY
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

    GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    delay();
    GPIO_WritePinOutput(GPIO1, 9, 1);

    /* Clock setting for LPI2C */
    CLOCK_SetMux(kCLOCK_Lpi2cMux, BOARD_ACCEL_I2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, BOARD_ACCEL_I2C_CLOCK_SOURCE_DIVIDER);
#endif
    CRYPTO_InitHardware();
#endif
    PRINTF("hello sfw.\r\n");

    sfw_main();
    
		return 0;
}
