/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FOTA_BASIC_DEMO_H_
#define __FOTA_BASIC_DEMO_H_

#include "aiot_mqtt_api.h"
#include "aiot_ota_api.h"

int demo_state_logcb(int32_t code, char *message);
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *const event, void *userdata);
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *const packet, void *userdata);
void user_download_recv_handler(void *handle, const aiot_download_recv_t *packet, void *userdata);
void user_ota_recv_handler(void *ota_handle, aiot_ota_recv_t *ota_msg, void *userdata);
int ota_main();

#endif