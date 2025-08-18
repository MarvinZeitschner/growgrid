#pragma once
#include "esp_err.h"

/**
 * @brief Initializes the I2C bus.
 *
 * @return ESP_OK on success.
 */
esp_err_t hal_i2c_init(void);
