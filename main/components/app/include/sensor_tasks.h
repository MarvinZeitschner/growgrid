#pragma once
#include "esp_err.h"

/**
 * @brief Starts the sensor reading tasks.
 *
 * This will create two tasks:
 * 1. A high-frequency task for temperature, humidity, and light.
 * 2. A low-frequency task for soil moisture.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_sensor_tasks_start(void);
