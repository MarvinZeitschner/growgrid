#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/queue.h"

esp_err_t soil_sensor_service_start(QueueHandle_t data_queue);
