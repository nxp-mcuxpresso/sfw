/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#include "usb_host_msd.h"
#include "board.h"
#include "fsl_common.h"
#include "board.h"

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

#include "usb_phy.h"
#include "clock_config.h"
#include "udisk_ota.h"

#include "ff.h"
#include "diskio.h"
#include "stdio.h"
#include "sysflash.h"
#include "bootutil_priv.h"
#include "flash_map.h"

#include "flexspi_flash.h"
#include "sbl_ota_flag.h"
#if defined(CONFIG_BOOT_ENCRYPTED_XIP)
#include "update_key_context.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_SIZE (4096U) 
#define VERSION32(hdr) (((hdr)->ih_ver.iv_major << 24) | ((hdr)->ih_ver.iv_minor << 16) | ((hdr)->ih_ver.iv_revision))

const char imageName_udisk[] = "newapp";
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief host callback function.
 *
 * device attach/detach callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The application don't support the configuration.
 */
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode);

/*!
 * @brief app initialization.
 */
static void USB_HostApplicationInit(void);

static void USB_HostTask(void *param);

static void USB_HostApplicationTask(void *param);

extern void USB_HostClockInit(void);
extern void USB_HostIsrEnable(void);
extern void USB_HostTaskFn(void *param);
void BOARD_InitHardware(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Allocate the memory for the heap. */
#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif

usb_host_handle g_HostHandle;

/*! @brief msd class handle array for fatfs */
extern usb_host_class_handle g_UsbFatfsClassHandle;

usb_host_msd_fatfs_instance_t g_MsdFatfsInstance; /* global msd fatfs instance */
static FATFS fatfs;
/* control transfer on-going state. It should set to 1 when start control transfer, it is set to 0 in the callback */
volatile uint8_t controlIng;
/* control transfer callback status */
volatile usb_status_t controlStatus;

static FIL g_fileObject;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t g_bufferRead[BUFFER_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/
void USB_HostMsdControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_msd_fatfs_instance_t *msdFatfsInstance = (usb_host_msd_fatfs_instance_t *)param;

    if (msdFatfsInstance->runWaitState == kUSB_HostMsdRunWaitSetInterface) /* set interface finish */
    {
        msdFatfsInstance->runWaitState = kUSB_HostMsdRunIdle;
        msdFatfsInstance->runState     = kUSB_HostMsdRunMassStorageTest;
    }
    controlIng    = 0;
    controlStatus = status;
}

usb_status_t USB_DiskUpgrade(usb_host_msd_fatfs_instance_t *msdFatfsInstance)
{
    char *image_title = NULL;
    FRESULT error;
    char *dir;
    int dir_len;
    FILINFO fileInformation;
    DIR directory = {0};
    uint8_t driverNumberBuffer[3];
    
    status_t status;
    volatile uint32_t primask;
    
    UINT bytesRead, buffer_size;
    struct image_version *cur_ver;
    struct image_header *new_hdr;
    uint8_t version_buff[8];
    int8_t cmp_result;
    uint8_t image_position;
    uint32_t dstAddr;
    
   sfw_flash_read(REMAP_FLAG_ADDRESS, &image_position, 1);
   if(image_position == 0x01)
   {
       dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, version_buff, 8);
        cur_ver = (struct image_version *)version_buff;
   }
   else if(image_position == 0x02)
   {
       dstAddr = FLASH_AREA_IMAGE_1_OFFSET;        
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_2_OFFSET + IMAGE_VERSION_OFFSET, version_buff, 8);
        cur_ver = (struct image_version *)version_buff;
   }
   else
   {
        return kStatus_USB_Error;
   }
    
    dir = ".";
    dir_len = strlen(dir);
    if ((dir[dir_len - 1] == '/') || (dir[dir_len - 1] == '\\'))
        dir[dir_len - 1] = '\0';
    
    usb_echo("U-Disk OTA test\r\n");

    usb_echo("fatfs mount as logiacal driver %d......", USBDISK);
    sprintf((char *)&driverNumberBuffer[0], "%c:", USBDISK + '0');
    error = f_mount(&fatfs, (char const *)&driverNumberBuffer[0], 0);
    if (error)
    {
        usb_echo("error\r\n");
        return kStatus_USB_Error;
    }
    usb_echo("success\r\n");

#if (FF_FS_RPATH >= 2)
    error = f_chdrive((char const *)&driverNumberBuffer[0]);
    if (error)
    {
        usb_echo("error\r\n");
        return kStatus_USB_Error;
    }
#endif

    error = f_opendir(&directory, dir/*(char const *)&driverNumberBuffer[0]*/);
    if (error)
    {
        return kStatus_USB_Error;
    }
    
    for (;;)
    {
        error = f_readdir(&directory, &fileInformation);
        if ((error != FR_OK) || (fileInformation.fname[0U] == 0U)) 
        {
            break;
        }
        if (fileInformation.fname[0] == '.') 
        {
            continue;
        }
#if 0        
        if (fileInformation.fattrib & AM_DIR) {
            PRINTF("\033[40;33m%s\033[0m \r\n", fileInformation.fname);
        } else {
            usb_echo("%s \r\n", fileInformation.fname);
        }
#endif   
            if (strncmp(fileInformation.fname, imageName_udisk, (sizeof(imageName_udisk)-1)) == 0)
            {
                image_title = fileInformation.fname;
                break;
            }
    }
    
    error = f_open(&g_fileObject, _T(image_title), FA_READ);
    if (error) 
    {
        goto exit;
    }
    
    usb_echo("reading...\r\n");
    
    buffer_size = sizeof(g_bufferRead);
    error = f_read(&g_fileObject, g_bufferRead, buffer_size, &bytesRead);
    if (error || (bytesRead != buffer_size) ) 
    {
        usb_echo("failed to read image data\r\n");
        goto exit;
    }
    
    new_hdr = (struct image_header *)g_bufferRead;
    if (new_hdr->ih_magic == IMAGE_MAGIC) 
    {
        if(cur_ver->iv_major == 0xff && cur_ver->iv_minor == 0xff)
        {
            usb_echo("new img verison: %d.%d.%d\r\n", new_hdr->ih_ver.iv_major,
                                new_hdr->ih_ver.iv_minor,
                                new_hdr->ih_ver.iv_revision);
        }
        else
        {
            cmp_result = compare_image_version(&new_hdr->ih_ver, cur_ver);
            if(cmp_result > 0)
            {
                usb_echo("new img verison: %d.%d.%d\r\n", new_hdr->ih_ver.iv_major,
                                                new_hdr->ih_ver.iv_minor,
                                                new_hdr->ih_ver.iv_revision);
            }
            else
            {
                usb_echo("The version number of the new image is not greater than the current image version number, please retry!\r\n");
                goto exit;
            }
        }
    } 
    else 
    {
        usb_echo("the new image is invalid.\r\n");
        goto exit;
    }
    
    primask = DisableGlobalIRQ();
    status = sfw_flash_erase(dstAddr, FLASH_AREA_IMAGE_2_SIZE);
    EnableGlobalIRQ(primask);
    if (status) 
    {
        usb_echo("erase failed.\r\n");
        goto exit;
    }


    usb_echo("updating...\r\n");
    
    primask = DisableGlobalIRQ();
    //already read the first 4KB for image validation checking, write to flash directly
    status = sfw_flash_write(dstAddr, (uint32_t *)g_bufferRead, bytesRead);  
    if (status) 
    {
        goto exit;
    }
    EnableGlobalIRQ(primask);
    
    while((bytesRead == buffer_size)) 
    {  
        dstAddr += bytesRead;
        error = f_read(&g_fileObject, (void *)g_bufferRead, buffer_size, &bytesRead);
        if (error == FR_OK) 
        {
            primask = DisableGlobalIRQ();
            status = sfw_flash_write(dstAddr, (uint32_t *)g_bufferRead, bytesRead);
            if (status) 
            {
                goto exit;
            }
            EnableGlobalIRQ(primask);
        }
        else 
        {
            goto exit;
        }
    }
    
    usb_echo("finished\r\n");
    
    write_update_type(UPDATE_TYPE_UDISK);
    enable_image();
#if defined(CONFIG_BOOT_ENCRYPTED_XIP)
    update_key_context();
#endif
    
    usb_echo("Please unplug the u-disk!\r\n");
    usb_echo("sys rst...\r\n\r\n");
    vTaskDelay(1000U);

#if 0
    primask = DisableGlobalIRQ();
    status = flexspi_nor_flash_reset_opi(EXAMPLE_FLEXSPI);
    if (status) 
    {
        goto exit;
    }
    EnableGlobalIRQ(primask);
#endif

    NVIC_SystemReset();
    
exit:
    //FLEXSPI_SoftwareReset(EXAMPLE_FLEXSPI); //bus fault
    if (f_close(&g_fileObject)) 
    {
        return kStatus_USB_Error;   
    }
    return kStatus_USB_Error;  
}
                
void USB_HostMsdTask(void *arg)
{
    usb_status_t status;
    usb_host_msd_fatfs_instance_t *msdFatfsInstance = (usb_host_msd_fatfs_instance_t *)arg;

    if (msdFatfsInstance->deviceState != msdFatfsInstance->prevDeviceState)
    {
        msdFatfsInstance->prevDeviceState = msdFatfsInstance->deviceState;
        switch (msdFatfsInstance->deviceState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached: /* deivce is attached and numeration is done */
                status                = USB_HostMsdInit(msdFatfsInstance->deviceHandle,
                                         &msdFatfsInstance->classHandle); /* msd class initialization */
                g_UsbFatfsClassHandle = msdFatfsInstance->classHandle;
                if (status != kStatus_USB_Success)
                {
                    usb_echo("usb host msd init fail\r\n");
                    return;
                }
                msdFatfsInstance->runState = kUSB_HostMsdRunSetInterface;
                break;

            case kStatus_DEV_Detached: /* device is detached */
                msdFatfsInstance->deviceState = kStatus_DEV_Idle;
                msdFatfsInstance->runState    = kUSB_HostMsdRunIdle;
                USB_HostMsdDeinit(msdFatfsInstance->deviceHandle,
                                  msdFatfsInstance->classHandle); /* msd class de-initialization */
                msdFatfsInstance->classHandle = NULL;

                usb_echo("mass storage device detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (msdFatfsInstance->runState)
    {
        case kUSB_HostMsdRunIdle:
            break;

        case kUSB_HostMsdRunSetInterface: /* set msd interface */
            msdFatfsInstance->runState     = kUSB_HostMsdRunIdle;
            msdFatfsInstance->runWaitState = kUSB_HostMsdRunWaitSetInterface;
            status = USB_HostMsdSetInterface(msdFatfsInstance->classHandle, msdFatfsInstance->interfaceHandle, 0,
                                             USB_HostMsdControlCallback, msdFatfsInstance);
            if (status != kStatus_USB_Success)
            {
                usb_echo("set interface fail\r\n");
            }
            break;

        case kUSB_HostMsdRunMassStorageTest: /* set interface succeed */
#if ((defined MSD_FATFS_THROUGHPUT_TEST_ENABLE) && (MSD_FATFS_THROUGHPUT_TEST_ENABLE))
            USB_HostMsdFatfsThroughputTest(msdFatfsInstance); /* test throughput */
#else
            USB_DiskUpgrade(msdFatfsInstance); /* test msd device */
#endif /* MSD_FATFS_THROUGHPUT_TEST_ENABLE */
            msdFatfsInstance->runState = kUSB_HostMsdRunIdle;
            break;

        default:
            break;
    }
}

usb_status_t USB_HostMsdEvent(usb_device_handle deviceHandle,
                              usb_host_configuration_handle configurationHandle,
                              uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    usb_host_configuration_t *configuration;
    uint8_t interfaceIndex;
    usb_host_interface_t *interface;
    uint32_t infoValue = 0;
    uint8_t id;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[interfaceIndex];
                id        = interface->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_MSD_CLASS_CODE)
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceSubClass;
                if ((id != USB_HOST_MSD_SUBCLASS_CODE_UFI) && (id != USB_HOST_MSD_SUBCLASS_CODE_SCSI))
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceProtocol;
                if (id != USB_HOST_MSD_PROTOCOL_BULK)
                {
                    continue;
                }
                else
                {
                    if (g_MsdFatfsInstance.deviceState == kStatus_DEV_Idle)
                    {
                        /* the interface is supported by the application */
                        g_MsdFatfsInstance.deviceHandle    = deviceHandle;
                        g_MsdFatfsInstance.interfaceHandle = interface;
                        g_MsdFatfsInstance.configHandle    = configurationHandle;
                        return kStatus_USB_Success;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_MsdFatfsInstance.configHandle == configurationHandle)
            {
                if ((g_MsdFatfsInstance.deviceHandle != NULL) && (g_MsdFatfsInstance.interfaceHandle != NULL))
                {
                    /* the device enumeration is done */
                    if (g_MsdFatfsInstance.deviceState == kStatus_DEV_Idle)
                    {
                        g_MsdFatfsInstance.deviceState = kStatus_DEV_Attached;

                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &infoValue);
                        usb_echo("mass storage device attached:pid=0x%x", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &infoValue);
                        usb_echo("vid=0x%x ", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &infoValue);
                        usb_echo("address=%d\r\n", infoValue);
                    }
                    else
                    {
                        usb_echo("not idle msd instance\r\n");
                        status = kStatus_USB_Error;
                    }
                }
            }
            break;

        case kUSB_HostEventDetach:
            if (g_MsdFatfsInstance.configHandle == configurationHandle)
            {
                /* the device is detached */
                g_UsbFatfsClassHandle           = NULL;
                g_MsdFatfsInstance.configHandle = NULL;
                if (g_MsdFatfsInstance.deviceState != kStatus_DEV_Idle)
                {
                    g_MsdFatfsInstance.deviceState = kStatus_DEV_Detached;
                }
            }
            break;

        default:
            break;
    }
    return status;
}


void USB_OTG1_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}

void USB_HostClockInit(void)
{
    uint32_t usbClockFreq;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
    usbClockFreq = 24000000;
    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
/* USB_HOST_CONFIG_EHCI */

/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}

/*!
 * @brief USB isr function.
 */

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    switch (eventCode & 0x0000FFFFU)
    {
        case kUSB_HostEventAttach:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventNotSupported:
            usb_echo("device not supported.\r\n");
            break;

        case kUSB_HostEventEnumerationDone:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventDetach:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventEnumerationFail:
            usb_echo("enumeration failed\r\n");
            break;

        default:
            break;
    }
    return status;
}

static void USB_HostApplicationInit(void)
{
    usb_status_t status = kStatus_USB_Success;

    USB_HostClockInit();

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    status = USB_HostInit(CONTROLLER_ID, &g_HostHandle, USB_HostEvent);
    if (status != kStatus_USB_Success)
    {
        usb_echo("host init error\r\n");
        return;
    }
    USB_HostIsrEnable();
    
    usb_echo("U-Disk updating task enable.\r\n");
}

static void USB_HostTask(void *param)
{
    vTaskDelay(500U);
    while (1)
    {
        USB_HostTaskFn(param);
        vTaskDelay(1U);
    }
}

static void USB_HostApplicationTask(void *param)
{
    vTaskDelay(500U);
    usb_echo("Please plug in a u-disk to board.\r\n");
    
    uint8_t last_update_type;
    
    sfw_flash_read(UPDATE_TYPE_FLAG_ADDRESS, &last_update_type, 1);
    
    if(last_update_type == UPDATE_TYPE_UDISK)
    {
        usb_echo("Update done, the last update type: %s\r\n",
               last_update_type == UPDATE_TYPE_SDCARD ? "SD Card" :
               last_update_type == UPDATE_TYPE_UDISK ? "U-Disk" :
               last_update_type == UPDATE_TYPE_AWS_CLOUD ? 
                       "AWS platform" : "BUG; can't happend");
        write_image_ok();        
    }
    while (1)
    {
        USB_HostMsdTask(param);
        vTaskDelay(1U);
    }
}

int udisk_ota_main(void)
{
    USB_HostApplicationInit();

    if (xTaskCreate(USB_HostTask, "usb host task", 2000L / sizeof(portSTACK_TYPE), g_HostHandle, 4, NULL) != pdPASS)
    {
        usb_echo("create host task error\r\n");
    }
    if (xTaskCreate(USB_HostApplicationTask, "app task", 2300L / sizeof(portSTACK_TYPE), &g_MsdFatfsInstance, 3,
                    NULL) != pdPASS)
    {
        usb_echo("create mouse task error\r\n");
    }

    return 0;
}
