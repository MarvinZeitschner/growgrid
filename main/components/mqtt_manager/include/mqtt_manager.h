#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/idf_additions.h"
#include "freertos/queue.h"
#include "sensor_data.h"
#include <stdbool.h>

/**
 * @brief Starts the MQTT client and the publisher task.
 *
 * @param queue The handle to the queue from which sensor data will be received.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t mqtt_manager_start(QueueSetHandle_t data_queue,
                             EventGroupHandle_t event_group,
                             SemaphoreHandle_t data_mutex,
                             SensorData_t *shared_data);

/**
 * @brief Checks if the MQTT client is currently connected.
 *
 * @return true if connected, false otherwise.
 */
bool mqtt_manager_is_connected(void);
