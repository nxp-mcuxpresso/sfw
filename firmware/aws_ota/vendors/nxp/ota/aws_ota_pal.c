/*
 * FreeRTOS OTA PAL V1.0.0
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.
 * Copyright 2021 NXP
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* OTA PAL implementation for NXP RT1xxx. */

#include <string.h>

#include "aws_iot_ota_pal.h"
#include "iot_crypto.h"
#include "aws_ota_codesigner_certificate.h"

#include "fsl_common.h"
#include "mflash_drv.h"

#include "sbl_ota_flag.h"
#include "flexspi_flash_config.h"
#include "flash_info.h"

#if defined(CONFIG_BOOT_ENCRYPTED_XIP)
#include "update_key_context.h"
#endif

/* Specify the OTA signature algorithm we support on this platform. */
const char cOTA_JSON_FileSignatureKey[OTA_FILE_SIG_KEY_STR_MAX_LENGTH] = "sig-sha256-ecdsa";


/* low level file context structure */
typedef struct
{
    const OTA_FileContext_t *FileXRef;
    uint32_t Addr;
    uint32_t Size;
    uint32_t MaxSize;
} LL_FileContext_t;

static LL_FileContext_t prvPAL_CurrentFileContext;

static char *prvPAL_GetCertificate(const uint8_t *pucCertName, uint32_t *ulSignerCertSize)
{
    if (NULL != ulSignerCertSize)
    {
        *ulSignerCertSize = sizeof(signingcredentialSIGNING_CERTIFICATE_PEM);
    }
    return (char *)signingcredentialSIGNING_CERTIFICATE_PEM;
}


static LL_FileContext_t *prvPAL_GetLLFileContext(OTA_FileContext_t *const C)
{
    LL_FileContext_t *FileContext;

    if ((C == NULL) || (C->pucFile == NULL))
    {
        return NULL;
    }

    FileContext = (LL_FileContext_t *)C->pucFile;

    if ((FileContext == NULL) || (FileContext->FileXRef != C))
    {
        return NULL;
    }

    return FileContext;
}


static OTA_Err_t prvPAL_CheckFileSignature(OTA_FileContext_t *const C)
{
    LL_FileContext_t *FileContext;
    uint8_t *FileData = NULL;

    void *VerificationContext;

    char *cert = NULL;
    uint32_t certsize;


    OTA_LOG_L1("[OTA-NXP] CheckFileSignature\r\n");

    FileContext = prvPAL_GetLLFileContext(C);
    if (FileContext == NULL)
    {
        return kOTA_Err_SignatureCheckFailed;
    }

    FileData = (void *)(FileContext->Addr + MFLASH_BASE_ADDRESS);
    if (FileData == NULL)
    {
        OTA_LOG_L1("[OTA-NXP] FileData == NULL\r\n");
        return kOTA_Err_SignatureCheckFailed;
    }

    cert = prvPAL_GetCertificate((const uint8_t *)C->pucCertFilepath, &certsize);
    if (cert == NULL)
    {
        OTA_LOG_L1("[OTA-NXP] cert == NULL\r\n");
        return kOTA_Err_BadSignerCert;
    }

    if (CRYPTO_SignatureVerificationStart(&VerificationContext, cryptoASYMMETRIC_ALGORITHM_ECDSA,
                                          cryptoHASH_ALGORITHM_SHA256) != pdTRUE)
    {
        OTA_LOG_L1("[OTA-NXP] kOTA_Err_SignatureCheckFailed1\r\n");
        return kOTA_Err_SignatureCheckFailed;
    }

    CRYPTO_SignatureVerificationUpdate(VerificationContext, FileData, FileContext->Size);

    if (CRYPTO_SignatureVerificationFinal(VerificationContext, cert, certsize, C->pxSignature->ucData,
                                          C->pxSignature->usSize) != pdTRUE)
    {
        OTA_LOG_L1("[OTA-NXP] kOTA_Err_SignatureCheckFailed2\r\n");
        return kOTA_Err_SignatureCheckFailed;
    }

    return kOTA_Err_None;
}


OTA_Err_t prvPAL_Abort(OTA_FileContext_t *const C)
{
    OTA_Err_t result = kOTA_Err_None;

    OTA_LOG_L1("[OTA-NXP] Abort\r\n");

    C->pucFile = NULL;
    return result;
}


OTA_Err_t prvPAL_CreateFileForRx(OTA_FileContext_t *const C)
{
    LL_FileContext_t *FileContext = &prvPAL_CurrentFileContext;

    OTA_LOG_L1("[OTA-NXP] CreateFileForRx\r\n");
    
    /* update partition address in FLASH memory and its size */
#ifdef SOC_REMAP_ENABLE
    uint8_t image_position;
    sfw_flash_read_ipc(REMAP_FLAG_ADDRESS, &image_position, 1);
    OTA_LOG_L1("[OTA-NXP] image_position= %d\r\n", image_position);
    if(image_position == 0x01)
    {
        if (C->ulFileSize > FLASH_AREA_IMAGE_2_SIZE)
        {
            return kOTA_Err_RxFileTooLarge;
        }
        FileContext->Addr = FLASH_AREA_IMAGE_2_OFFSET;
        FileContext->MaxSize = FLASH_AREA_IMAGE_2_SIZE;
    }
    else if(image_position == 0x02)
    {
        if (C->ulFileSize > FLASH_AREA_IMAGE_1_SIZE)
        {
            return kOTA_Err_RxFileTooLarge;
        }
        FileContext->Addr = FLASH_AREA_IMAGE_1_OFFSET;
        FileContext->MaxSize = FLASH_AREA_IMAGE_1_SIZE;
    }
    else
    {
        return kStatus_Fail;
    }
#else
    FileContext->Addr = FLASH_AREA_IMAGE_2_OFFSET;
    FileContext->MaxSize = FLASH_AREA_IMAGE_2_SIZE;
#endif

    OTA_LOG_L1("[OTA-NXP] File_Addr = 0x%08x, File_Size = 0x%08x\r\n", FileContext->Addr, FileContext->MaxSize);

    /* actual size of the file according to data received */
    FileContext->Size     = 0;

    FileContext->FileXRef = C; /* pointer cross reference for integrity check */
    C->pucFile = (uint8_t *)FileContext;

    return kOTA_Err_None;
}


OTA_Err_t prvPAL_CloseFile(OTA_FileContext_t *const C)
{
    OTA_Err_t result = kOTA_Err_None;
    LL_FileContext_t *FileContext;
    uint8_t *FileData = NULL;
#ifdef SOC_REMAP_ENABLE
#ifdef SOC_IMXRT1170_SERIES
    uint32_t remap_offset = (*(uint32_t *)0x400CC428) & 0xFFFFF000;
#else //RT1060, RT1064
    uint32_t remap_offset = IOMUXC_GPR->GPR32 & 0xFFFFF000;
#endif
    uint8_t image_position;
#endif
    struct image_version *cur_ver;
    struct image_version *new_ver;
    uint8_t cur_version[8];
    uint8_t new_version[8];
    int8_t cmp_result;
    volatile uint32_t primask;

    OTA_LOG_L1("[OTA-NXP] CloseFile\r\n");

    FileContext = prvPAL_GetLLFileContext(C);
    if (FileContext == NULL)
    {
        return kOTA_Err_FileClose;
    }

#ifdef SOC_REMAP_ENABLE
    //should disable remap when check signature.
    if (remap_offset != 0)
    {
        SBL_DisableRemap();
    }
#endif

    result = prvPAL_CheckFileSignature(C);
    if (result != kOTA_Err_None)
    {
        OTA_LOG_L1("[OTA-NXP] CheckFileSignature failed\r\n");
        return result;
    }

    FileData = (void *)(FileContext->Addr + MFLASH_BASE_ADDRESS);
    if (FileData == NULL)
    {
        return kOTA_Err_BootInfoCreateFailed;
    }

#ifdef SOC_REMAP_ENABLE
    sfw_flash_read_ipc(REMAP_FLAG_ADDRESS, &image_position, 1);
    OTA_LOG_L1("[prvPAL_CloseFile] image_position= %d\r\n", image_position); 
    if(image_position == 0x01)
    {
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, cur_version, 8);
        cur_ver = (struct image_version *)cur_version;
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_2_OFFSET + IMAGE_VERSION_OFFSET, new_version, 8);
        new_ver = (struct image_version *)new_version;
    }
    else if(image_position == 0x02)
    {
        //after check, enable remap
        SBL_EnableRemap(BOOT_FLASH_ACT_APP, BOOT_FLASH_ACT_APP+FLASH_AREA_IMAGE_1_SIZE, FLASH_AREA_IMAGE_1_SIZE);
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_2_OFFSET + IMAGE_VERSION_OFFSET, cur_version, 8);
        cur_ver = (struct image_version *)cur_version;
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, new_version, 8);
        new_ver = (struct image_version *)new_version;
    }
    else
    {
        return kOTA_Err_RxFileCreateFailed;
    }
#else
    sfw_flash_read(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, cur_version, 8);
    cur_ver = (struct image_version *)cur_version;
    sfw_flash_read(FLASH_AREA_IMAGE_2_OFFSET + IMAGE_VERSION_OFFSET, new_version, 8);
    new_ver = (struct image_version *)new_version;
#endif
    
    //check image version
    cmp_result = compare_image_version(new_ver, cur_ver);
    OTA_LOG_L1("[OTA-NXP] cmp_result=%d\r\n", cmp_result);
    if(cmp_result > 0)
    {
        OTA_LOG_L1("[OTA-NXP] new image verison: %d.%d.%d\r\n", new_ver->iv_major, new_ver->iv_minor, new_ver->iv_revision);
    }
    else
    {
        OTA_LOG_L1("[OTA-NXP] The version number of the new image is not greater than the current image version number!\r\n");
        return kOTA_Err_RxFileCreateFailed;
    }

    C->pucFile = NULL;
    return result;
}


int16_t prvPAL_WriteBlock(OTA_FileContext_t *const C, uint32_t ulOffset, uint8_t *const pcData, uint32_t ulBlockSize)
{
    int32_t result;
    LL_FileContext_t *FileContext;

    OTA_LOG_L1("[OTA-NXP] WriteBlock %x : %x\r\n", ulOffset, ulBlockSize);

    FileContext = prvPAL_GetLLFileContext(C);
    if (FileContext == NULL)
    {
        return -1;
    }

    if (ulOffset + ulBlockSize > FileContext->MaxSize)
    {
        return -1;
    }

    result = mflash_drv_write(FileContext->Addr + ulOffset, pcData, ulBlockSize);
    if (result == 0)
    {
        /* zero indicates no error, return number of bytes written to the caller */
        result = ulBlockSize;
        if (FileContext->Size < ulOffset + ulBlockSize)
        {
            /* extend file size according to highest offset */
            FileContext->Size = ulOffset + ulBlockSize;
        }
    }
    return result;
}


OTA_Err_t prvPAL_ActivateNewImage(void)
{
    OTA_LOG_L1("[OTA-NXP] Write update type\r\n");
    write_update_type(UPDATE_TYPE_AWS_CLOUD);
    OTA_LOG_L1("[OTA-NXP] Write image trailer\r\n");
    enable_image();
#if defined(CONFIG_BOOT_ENCRYPTED_XIP)
    update_key_context();
#endif
    OTA_LOG_L1("[OTA-NXP] ActivateNewImage\r\n");
    prvPAL_ResetDevice(); /* go for reboot */
    return kOTA_Err_None;
}


OTA_Err_t prvPAL_ResetDevice(void)
{
    OTA_LOG_L1("[OTA-NXP] ResetDevice\r\n");
    NVIC_SystemReset(); /* this should never return */
}


OTA_Err_t prvPAL_SetPlatformImageState(OTA_ImageState_t eState)
{
    OTA_Err_t result = kOTA_Err_None;
    uint8_t ota_type;

    OTA_LOG_L1("[OTA-NXP] SetPlatformImageState %d\r\n", eState);

    if (prvPAL_GetPlatformImageState() == eOTA_PAL_ImageState_PendingCommit)
    {
        /* Device in test mode */
        switch (eState)
        {
            case eOTA_ImageState_Accepted:
                /* iamge is ok */
                sfw_flash_read(UPDATE_TYPE_FLAG_ADDRESS, &ota_type, 1);
                if (UPDATE_TYPE_AWS_CLOUD == ota_type)
                {
                    write_image_ok();
                }
                break;

            case eOTA_ImageState_Rejected:
                /* Invalidate the image */
                break;

            case eOTA_ImageState_Aborted:
                /* Invalidate the image */
                break;

            case eOTA_ImageState_Testing:
                result = kOTA_Err_None;
                break;

            default:
                result = kOTA_Err_BadImageState;
                break;
        }
    }
    else
    {
        /* Normal mode */
        switch (eState)
        {
            case eOTA_ImageState_Accepted:
                /* No pending commit */
                result = kOTA_Err_CommitFailed;
                break;

            case eOTA_ImageState_Rejected:
                result = kOTA_Err_RejectFailed;
                break;

            case eOTA_ImageState_Aborted:
                result = kOTA_Err_AbortFailed;
                break;

            case eOTA_ImageState_Testing:
                result = kOTA_Err_BadImageState;
                break;

            default:
                result = kOTA_Err_BadImageState;
                break;
        }
    }

    return result;
}


OTA_PAL_ImageState_t prvPAL_GetPlatformImageState(void)
{
    uint8_t ota_status = 0;
    
    OTA_LOG_L1("[OTA-NXP] GetPlatformImageState\r\n");

    ota_status = read_ota_status();
    OTA_LOG_L1("[OTA-NXP] ota_status = 0x%x\r\n", ota_status);

    if (ota_status == 0x00)
    {
        return eOTA_PAL_ImageState_Valid;
    }
    else if (ota_status == 0x01)
    {
        return eOTA_PAL_ImageState_PendingCommit;
    }
    else
    {
        return eOTA_PAL_ImageState_Invalid;
    }
}


/* Provide access to private members for testing. */
#ifdef AMAZON_FREERTOS_ENABLE_UNIT_TESTS
#include "aws_ota_pal_test_access_define.h"
#endif
