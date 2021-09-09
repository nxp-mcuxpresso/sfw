// ############################################################################
// #                       Copyright 2017 - 2019 NXP                          #
// #                       (C), NXP B.V. of Eindhoven                         #
// #                                                                          #
// # All rights are reserved. Reproduction in whole or in part is prohibited  #
// # without the written consent of the copyright owner.                      #
// # NXP reserves the right to make changes without notice at any time.       #
// # NXP makes no warranty, expressed, implied or statutory, including but    #
// # not limited to any implied warranty of merchantability or fitness for    #
// # any particular purpose, or that the use will not infringe any third      #
// # party patent, copyright or trademark. NXP must not be liable for any     #
// # loss or damage arising from its use.                                     #
// ############################################################################

#ifndef __PD_A1006_MASTER_H__
#define __PD_A1006_MASTER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ENABLE_A100X_TEST_CA_SUPPORT

/* Replace with diversified Host UID (up to 16 bytes) */
#define HOST_UID "A1006 Host UID"

#define I2C_SLAVE_ADDR_SHIELD1 0x60 /* 8-bit slave address of A1006 */
#define I2C_SLAVE_ADDR_SHIELD2 0x50 /* 8-bit slave address of A1006 */

/*! @brief Minimum size of interface buffer when downloading root certificate. */
#define A1006_IFBUF_DLCERT_SIZE 148 /* 148=(16+128)+2*2 */

typedef struct _usb_pd_a1006_config
{
    uint8_t a1006SlaveAddress;
} usb_pd_a1006_config;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif
