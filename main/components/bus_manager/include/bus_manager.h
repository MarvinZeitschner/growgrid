#pragma once

#include "esp_err.h"
#include "i2c_bus.h"

/**
 * @brief Initializes the I2C bus.
 * @return ESP_OK on success.
 */
esp_err_t bus_manager_init_i2c(void);

/**
 * @brief Gets the handle to the initialized I2C bus.
 * @return Handle to the I2C bus. Returns NULL if not initialized.
 */
i2c_bus_handle_t bus_manager_get_i2c_handle(void);
