#pragma once
#include "esp_err.h"

/**
 * @brief Starts the pump control task.
 *
 * This task listens for soil moisture events and controls the pump accordingly.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_pump_control_task_start(void);
