/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sfw.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "sbl_ota_flag.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
 
 #ifdef OTA_SUPPORT
uint32_t i = 0;
void vApplicationIdleHook( void )
{
    if(i <= (uint32_t)(pdMS_TO_TICKS(3000U)))
    {
        i++;
    }
    if(i == (uint32_t)(pdMS_TO_TICKS(3000U)))
    {
        PRINTF("idle ctr 3 sec.\r\n");
        write_image_ok();
        //PRINTF("App1 is running - RT1020\r\n\r\n");
        PRINTF("The updating mode is remap, update successfully, App2 is running now - RT1020\r\n\r\n");
    }
} 
#endif

/*!
 * @brief Main function
 */
int main(void)
{
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("hello sfw.\r\n");

    sfw_main();
	
	return 0;
}

int memset_s(void *s, size_t smax, int c, size_t n)
{
    if (n > smax)
    {
        return 1;
    }
    memset(s, c, n);
    return 0;
}

