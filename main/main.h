//version comments
/*
  implements ble simple control  
*/

#ifndef MAIN_H
#define MAIN_H

#define my_app_cpu 1

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include <stdint.h>

#define CMD_TAG "CMD"

/*
Tasks that will run
*/
static TaskHandle_t hardware_control_task = NULL;

/*
global entities
*/
extern QueueHandle_t command_queue;



#endif