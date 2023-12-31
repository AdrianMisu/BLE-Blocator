/* board.c - Board-specific hooks */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */



#include "driver/gpio.h"
#include "esp_log.h"
#include "board.h"

#define TAG "BOARD"


void board_print()
{
    printf("Device is initialized\n");
    ESP_LOGI(TAG,"Device is initialized");
}



void board_init(void)
{
    board_print();
}
