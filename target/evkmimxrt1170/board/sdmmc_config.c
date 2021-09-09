/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdmmc_config.h"
#include "fsl_iomuxc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_SDCardPowerControl(bool enable);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*!brief sdmmc dma buffer */
AT_NONCACHEABLE_SECTION_ALIGN(static uint32_t s_sdmmcHostDmaBuffer[BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE],
                              SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);
#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
static sd_detect_card_t s_cd;
static sd_io_voltage_t s_ioVoltage = {
    .type = BOARD_SDMMC_SD_IO_VOLTAGE_CONTROL_TYPE,
    .func = NULL,
};
#endif
static sdmmchost_t s_host;
OSA_EVENT_HANDLE_DEFINE(s_event);
#ifdef SDIO_ENABLED
static sdio_card_int_t s_sdioInt;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
uint32_t BOARD_USDHC1ClockConfiguration(void)
{
    clock_root_config_t rootCfg = {0};
    /* SYS PLL2 528MHz. */
    const clock_sys_pll_config_t sysPllConfig = {
        .loopDivider = 1,
        /* Using 24Mhz OSC */
        .mfn = 0,
        .mfi = 22,
    };

    CLOCK_InitSysPll2(&sysPllConfig);
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd2, 24);

    rootCfg.mux = 4;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);

    return CLOCK_GetRootClockFreq(kCLOCK_Root_Usdhc1);
}

#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
bool BOARD_SDCardGetDetectStatus(void)
{
    return GPIO_PinRead(BOARD_SDMMC_SD_CD_GPIO_BASE, BOARD_SDMMC_SD_CD_GPIO_PIN) == BOARD_SDMMC_SD_CD_INSERT_LEVEL;
}

void BOARD_SDMMC_SD_CD_PORT_IRQ_HANDLER(void)
{
    if (GPIO_PortGetInterruptFlags(BOARD_SDMMC_SD_CD_GPIO_BASE) & (1U << BOARD_SDMMC_SD_CD_GPIO_PIN))
    {
        if (s_cd.callback != NULL)
        {
            s_cd.callback(BOARD_SDCardGetDetectStatus(), s_cd.userData);
        }
    }
    /* Clear interrupt flag.*/
    GPIO_PortClearInterruptFlags(BOARD_SDMMC_SD_CD_GPIO_BASE, ~0U);
}

//void BOARD_SDCardDAT3PullFunction(uint32_t status)
//{
//    if (status == kSD_DAT3PullDown)
//    {
//        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3,
//                            0xCU); /* no pull in IOMUX configuration, 100K pull down externally is required. */
//    }
//    else
//    {
//        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 0x4U);
//    }
//}

void BOARD_SDCardDetectInit(sd_cd_t cd, void *userData)
{
    /* install card detect callback */
    s_cd.cdDebounce_ms = BOARD_SDMMC_SD_CARD_DETECT_DEBOUNCE_DELAY_MS;
    s_cd.type          = BOARD_SDMMC_SD_CD_TYPE;
    s_cd.cardDetected  = BOARD_SDCardGetDetectStatus;
    s_cd.callback      = cd;
    s_cd.userData      = userData;

    if (BOARD_SDMMC_SD_CD_TYPE == kSD_DetectCardByGpioCD)
    {
        gpio_pin_config_t sw_config = {
            kGPIO_DigitalInput,
            0,
            kGPIO_IntRisingOrFallingEdge,
        };
        GPIO_PinInit(BOARD_SDMMC_SD_CD_GPIO_BASE, BOARD_SDMMC_SD_CD_GPIO_PIN, &sw_config);
        GPIO_PortEnableInterrupts(BOARD_SDMMC_SD_CD_GPIO_BASE, 1U << BOARD_SDMMC_SD_CD_GPIO_PIN);

        /* set IRQ priority */
        NVIC_SetPriority(BOARD_SDMMC_SD_CD_IRQ, BOARD_SDMMC_SD_CD_IRQ_PRIORITY);
        GPIO_PortClearInterruptFlags(BOARD_SDMMC_SD_CD_GPIO_BASE, ~0);
        /* Open card detection pin NVIC. */
        EnableIRQ(BOARD_SDMMC_SD_CD_IRQ);

        if (GPIO_PinRead(BOARD_SDMMC_SD_CD_GPIO_BASE, BOARD_SDMMC_SD_CD_GPIO_PIN) == BOARD_SDMMC_SD_CD_INSERT_LEVEL)
        {
            if (cd != NULL)
            {
                cd(true, userData);
            }
        }
    }

//    /* register DAT3 pull function switch function pointer */
//    if (BOARD_SDMMC_SD_CD_TYPE == kSD_DetectCardByHostDATA3)
//    {
//        s_cd.dat3PullFunc = BOARD_SDCardDAT3PullFunction;
//        /* make sure the card is power on for DAT3 pull up */
//        BOARD_SDCardPowerControl(true);
//    }
}

void BOARD_SDCardPowerResetInit(void)
{
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalOutput,
        1,
        kGPIO_NoIntmode,
    };
    GPIO_PinInit(BOARD_SDMMC_SD_POWER_RESET_GPIO_BASE, BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN, &sw_config);
}

void BOARD_SDCardPowerControl(bool enable)
{
    if (enable)
    {
        GPIO_PinWrite(BOARD_SDMMC_SD_POWER_RESET_GPIO_BASE, BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN, 0);
    }
    else
    {
        /* Power off the card only when the card is inserted, since the card detect circuit is depend on the power on
         * the EVK, card detect will not work if the power is off */
        if (BOARD_SDCardGetDetectStatus() == true)
        {
            GPIO_PinWrite(BOARD_SDMMC_SD_POWER_RESET_GPIO_BASE, BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN, 1);
        }
    }
}
#endif

#ifdef SD_ENABLED
void BOARD_SD_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, void *userData)
{
    assert(card);

    s_host.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
//#if (defined __DCACHE_PRESENT) && __DCACHE_PRESENT
//    s_host.enableCacheControl = BOARD_SDMMC_HOST_CACHE_CONTROL;
//#endif

    ((sd_card_t *)card)->host                                = &s_host;
    ((sd_card_t *)card)->host->hostController.base           = BOARD_SDMMC_SD_HOST_BASEADDR;
    ((sd_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();

    ((sd_card_t *)card)->host->hostEvent     = &s_event;
    ((sd_card_t *)card)->usrParam.cd         = &s_cd;
    ((sd_card_t *)card)->usrParam.pwr        = BOARD_SDCardPowerControl;
    ((sd_card_t *)card)->usrParam.ioStrength = NULL;
    ((sd_card_t *)card)->usrParam.ioVoltage  = &s_ioVoltage;
    ((sd_card_t *)card)->usrParam.maxFreq    = BOARD_SDMMC_SD_HOST_SUPPORT_SDR104_FREQ;

    BOARD_SDCardPowerResetInit();

    BOARD_SDCardDetectInit(cd, userData);

    NVIC_SetPriority(BOARD_SDMMC_SD_HOST_IRQ, hostIRQPriority);
}
#endif

#ifdef SDIO_ENABLED
void BOARD_SDIO_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, sdio_int_t cardInt)
{
    assert(card);

    s_host.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
#if (defined __DCACHE_PRESENT) && __DCACHE_PRESENT
    s_host.enableCacheControl = BOARD_SDMMC_HOST_CACHE_CONTROL;
#endif

    ((sdio_card_t *)card)->host                                = &s_host;
    ((sdio_card_t *)card)->host->hostController.base           = BOARD_SDMMC_SDIO_HOST_BASEADDR;
    ((sdio_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();

    ((sdio_card_t *)card)->usrParam.cd         = &s_cd;
    ((sdio_card_t *)card)->usrParam.pwr        = BOARD_SDCardPowerControl;
    ((sdio_card_t *)card)->usrParam.ioStrength = NULL;
    ((sdio_card_t *)card)->usrParam.ioVoltage  = &s_ioVoltage;
    ((sdio_card_t *)card)->usrParam.maxFreq    = BOARD_SDMMC_SD_HOST_SUPPORT_SDR104_FREQ;
    if (cardInt != NULL)
    {
        s_sdioInt.cardInterrupt                 = cardInt;
        ((sdio_card_t *)card)->usrParam.sdioInt = &s_sdioInt;
    }

    BOARD_SDCardPowerResetInit();
    BOARD_SDCardDetectInit(cd, NULL);

    NVIC_SetPriority(BOARD_SDMMC_SDIO_HOST_IRQ, hostIRQPriority);
}
#endif

#ifdef MMC_ENABLED
void BOARD_MMC_Config(void *card, uint32_t hostIRQPriority)
{
    assert(card);

    s_host.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
#if (defined __DCACHE_PRESENT) && __DCACHE_PRESENT
    s_host.enableCacheControl = BOARD_SDMMC_HOST_CACHE_CONTROL;
#endif

    ((mmc_card_t *)card)->host                                = &s_host;
    ((mmc_card_t *)card)->host->hostController.base           = BOARD_SDMMC_MMC_HOST_BASEADDR;
    ((mmc_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();
    ((mmc_card_t *)card)->host->tuningType                    = BOARD_SDMMC_MMC_TUNING_TYPE;
    ((mmc_card_t *)card)->usrParam.ioStrength                 = NULL;
    ((mmc_card_t *)card)->usrParam.maxFreq                    = BOARD_SDMMC_MMC_HOST_SUPPORT_HS200_FREQ;

    ((mmc_card_t *)card)->usrParam.capability |= BOARD_SDMMC_MMC_SUPPORT_8_BIT_DATA_WIDTH;

    ((mmc_card_t *)card)->hostVoltageWindowVCC  = BOARD_SDMMC_MMC_VCC_SUPPLY;
    ((mmc_card_t *)card)->hostVoltageWindowVCCQ = BOARD_SDMMC_MMC_VCCQ_SUPPLY;

    NVIC_SetPriority(BOARD_SDMMC_MMC_HOST_IRQ, hostIRQPriority);
}
#endif
