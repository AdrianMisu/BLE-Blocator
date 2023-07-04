/* board.h - Board-specific hooks */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "driver/gpio.h"

#if defined(CONFIG_BLE_MESH_ESP_WROOM_32)
#elif defined(CONFIG_BLE_MESH_ESP_WROVER)
#elif defined(CONFIG_BLE_MESH_ESP32C3_DEV)
#elif defined(CONFIG_BLE_MESH_ESP32S3_DEV)
#endif


void board_print();

void board_init(void);

#endif
