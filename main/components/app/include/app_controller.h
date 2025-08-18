#pragma once
#include "esp_err.h"

/**
 * @brief Initializes and starts the entire application.
 *
 * This is the main entry point for the application logic.
 * It initializes all modules and starts all tasks.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_controller_init(void);
