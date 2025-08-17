#pragma once
#include "esp_err.h"

/**
 * @brief Initializes the pump GPIO.
 * @return ESP_OK on success.
 */
esp_err_t hal_pump_init(void);

/**
 * @brief Turns the pump on.
 * @return ESP_OK on success.
 */
esp_err_t hal_pump_on(void);

/**
 * @brief Turns the pump off.
 * @return ESP_OK on success.
 */
esp_err_t hal_pump_off(void);
