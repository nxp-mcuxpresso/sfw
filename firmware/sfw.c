/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sfw.h>
/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "fsl_debug_console.h"

#ifdef OTA_SUPPORT
#include "sbl_ota_flag.h"
#ifdef SOC_LPC55S69_SERIES
#include "iap_flash_ops.h"
#else
#include "flexspi_flash_config.h"
#endif
#endif


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define hello_task1_PRIORITY (configMAX_PRIORITIES - 4)
#define hello_task2_PRIORITY (configMAX_PRIORITIES - 5)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task1(void *pvParameters);
static void hello_task2(void *pvParameters);
void timer(void *pvParameters);
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application main task entry point called by the main() in os abstract layer.
 */

int sfw_main(void)
{
    print_image_version();
#ifdef OTA_SUPPORT
    sfw_flash_init();
#endif
    
#ifdef OTA_SDCARD
    sdcard_ota_main();
#endif
 
#ifdef OTA_UDISK
    udisk_ota_main();
#endif
  
#ifdef OTA_AWS
    aws_main();
#elif defined(OTA_ALIYUN)
    aliyun_main();
#endif

    if (xTaskCreate(hello_task1, "Hello_task1", configMINIMAL_STACK_SIZE + 100, NULL, hello_task1_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Hello world1 task creation failed!.\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(hello_task2, "Hello_task2", configMINIMAL_STACK_SIZE + 100, NULL, hello_task2_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Hello world2 task creation failed!.\r\n");
        while (1)
            ;
    }

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* Scheduler should never reach this point. */
    while (true)
    {
    }
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void hello_task1(void *pvParameters)
{
    for (;;)
    {
        PRINTF("Hello world1.\r\n");
        vTaskDelay(1000);
    }
}

static void hello_task2(void *pvParameters)
{
    for (;;)
    {
        PRINTF("Hello world2.\r\n");
        vTaskDelay(1000);
    }
}
