#pragma once
#include "esp_err.h"

/**
 * @brief Starts the sensor reading tasks.
 *
 * This will create a separate FreeRTOS task for each sensor.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_sensor_tasks_start(void);
