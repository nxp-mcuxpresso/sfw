/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_RAND_H_
#define _USB_RAND_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

/* USB rand utility initialize */
void USB_RandInit(uint8_t instance, uint32_t seed);

/* USB rand utility de-initialize */
void USB_RandDeinit(uint8_t instance);

/* Get rand value from USB rand utility */
uint8_t USB_RandGet(uint8_t instance);

/* Get rand multi data from USB rand utility
 * return 1 if success; return 0 if fail.
 */
uint8_t USB_RandGetMulti(uint8_t instance, uint8_t *data, uint32_t count);

#endif /* _USB_RAND_H_ */
