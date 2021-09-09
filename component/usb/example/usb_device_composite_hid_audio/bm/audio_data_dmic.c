/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016,2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_dmic.h"
#include "fsl_dma.h"
#include "fsl_dmic_dma.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_audio_config.h"
#include "usb_device_descriptor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_LENGTH 982U
#define FIFO_DEPTH 8U
#if defined(FSL_FEATURE_SOC_USBHSH_COUNT) && (FSL_FEATURE_SOC_USBHSH_COUNT > 0U)
#define EXAMPLE_DMA DMA0
#define DMAREQ_DMIC 17
#define APP_DMIC_CHANNEL kDMIC_Channel1
#define APP_DMIC_CHANNEL_ENABLE DMIC_CHANEN_EN_CH1(1)
#else
#define EXAMPLE_DMA DMA0
#define DMAREQ_DMIC 16
#define APP_DMIC_CHANNEL kDMIC_Channel0
#define APP_DMIC_CHANNEL_ENABLE DMIC_CHANEN_EN_CH0(1)
#endif

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#define AUDIO_ENDPOINT_MAX_PACKET_SIZE \
    (FS_ISO_IN_ENDP_PACKET_SIZE > HS_ISO_IN_ENDP_PACKET_SIZE ? FS_ISO_IN_ENDP_PACKET_SIZE : HS_ISO_IN_ENDP_PACKET_SIZE)
#endif

#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define AUDIO_ENDPOINT_MAX_PACKET_SIZE (FS_ISO_IN_ENDP_PACKET_SIZE)
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define AUDIO_ENDPOINT_MAX_PACKET_SIZE (FS_ISO_IN_ENDP_PACKET_SIZE)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_DMA_NONINIT_DATA_ALIGN(4) unsigned short g_data_buffer[BUFFER_LENGTH] = {0};
dmic_dma_handle_t g_dmicDmaHandle;
dma_handle_t g_dmicRxDmaHandle;
volatile unsigned int first_int = 0;
dma_handle_t g_DMA_Handle; /*!< The DMA RX Handles. */

/*! @brief Static table of descriptors */
#if defined(__ICCARM__)
#pragma data_alignment              = 16U
dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__CC_ARM) || (defined(__ARMCC_VERSION))
__attribute__((aligned(16U))) dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__GNUC__)
__attribute__((aligned(16U))) dma_descriptor_t g_pingpong_desc[2] = {0};
#endif

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t s_wavBuff[AUDIO_ENDPOINT_MAX_PACKET_SIZE];
uint32_t audioPosition = 0U;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Audio wav data prepare function.
 *
 * This function prepare audio wav data before send.
 */
void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size)
{
    uint8_t k;
    /* copy audio wav data from flash to buffer */
    for (k = 0U; k < size; k++)
    {
        if (audioPosition > (BUFFER_LENGTH - 1U))
        {
            audioPosition = 0U;
        }
        if ((k & 0x1U) == 1)
        {
            *(buffer + k) = g_data_buffer[audioPosition] >> 8U;
            audioPosition++;
        }
        else
            *(buffer + k) = g_data_buffer[audioPosition];
    }
}

/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (tcds == kDMA_IntB)
    {
    }
    if (tcds == kDMA_IntA)
    {
    }
    if (first_int == 0U)
    {
        audioPosition = 0U;
        first_int     = 1U;
    }
}

void Board_DMIC_DMA_Init(void)
{
    dmic_channel_config_t dmic_channel_cfg;
    dma_transfer_config_t transferConfig;

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = DEMO_DMIC_OSR;
    dmic_channel_cfg.gainshft            = 2U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 1;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
    DMIC_Init(DMIC0);

    DMIC_ConfigIO(DMIC0, kDMIC_PdmDual);
    DMIC_Use2fs(DMIC0, true);
    DMIC_SetOperationMode(DMIC0, kDMIC_OperationModeDma);
    DMIC_ConfigChannel(DMIC0, APP_DMIC_CHANNEL, kDMIC_Left, &dmic_channel_cfg);

    DMIC_FifoChannel(DMIC0, APP_DMIC_CHANNEL, FIFO_DEPTH, true, true);

    DMIC_EnableChannnel(DMIC0, APP_DMIC_CHANNEL_ENABLE);

    DMA_Init(EXAMPLE_DMA);

    DMA_EnableChannel(EXAMPLE_DMA, DMAREQ_DMIC);

    /* Request dma channels from DMA manager. */
    DMA_CreateHandle(&g_DMA_Handle, EXAMPLE_DMA, DMAREQ_DMIC);

    DMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    DMA_PrepareTransfer(&transferConfig, (void *)&DMIC0->CHANNEL[APP_DMIC_CHANNEL].FIFO_DATA, g_data_buffer, 2,
                        BUFFER_LENGTH, kDMA_PeripheralToMemory, &g_pingpong_desc[1]);
    DMA_SubmitTransfer(&g_DMA_Handle, &transferConfig);
    transferConfig.xfercfg.intA = false;
    transferConfig.xfercfg.intB = true;
    DMA_CreateDescriptor(&g_pingpong_desc[1], &transferConfig.xfercfg,
                         (void *)&DMIC0->CHANNEL[APP_DMIC_CHANNEL].FIFO_DATA, &g_data_buffer[BUFFER_LENGTH / 2],
                         &g_pingpong_desc[0]);
    transferConfig.xfercfg.intA = true;
    transferConfig.xfercfg.intB = false;
    DMA_CreateDescriptor(&g_pingpong_desc[0], &transferConfig.xfercfg,
                         (void *)&DMIC0->CHANNEL[APP_DMIC_CHANNEL].FIFO_DATA, &g_data_buffer[0], &g_pingpong_desc[1]);
    DMA_StartTransfer(&g_DMA_Handle);
}
