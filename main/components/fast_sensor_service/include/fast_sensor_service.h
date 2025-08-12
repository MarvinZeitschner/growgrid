#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/idf_additions.h"
#include "freertos/queue.h"
#include "sensor_data.h"

esp_err_t fast_sensor_service_start(QueueHandle_t data_queue,
                                    EventGroupHandle_t event_group,
                                    SemaphoreHandle_t data_mutex,
                                    SensorData_t *shared_sensor_data);
