#ifndef I2C_H
#define I2C_H

#include "driver/i2c_types.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
#include "soc/gpio_num.h"
#include <stdint.h>

/**
 * @brief Initializes ESP32 as master for I2C coms
 *
 * @param port: Port number (e.g. I2C_NUM_0)
 * @param sda: GPIO pin used for sda
 * @param scl: GPIO pin used for scl
 * @param bus_handle: Pointer to variable that saves I2C bus handle
 *
 * @returns
 *    - ESP_OK Success
 *    - ESP_ERR_INVALID_ARG Parameter error
 *    - ESP_FAIL Driver installation error/
 */
esp_err_t i2c_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl,
                   i2c_master_bus_handle_t *bus_handle);

/**
 * @brief Add a devices to the I2C bus
 *
 * @param port: Port number (e.g. I2C_NUM_0)
 *
 * @returns
 *     - ESP_OK: I2C master bus initialized successfully.
 *     - ESP_ERR_INVALID_ARG: I2C bus initialization failed because of invalid
 * argument.
 *     - ESP_ERR_NO_MEM: Create I2C bus failed because of out of memory.
 *     - ESP_ERR_NOT_FOUND: No more free bus.
 */
esp_err_t i2c_add_device(i2c_master_bus_handle_t bus_handle,
                         uint8_t device_address, uint32_t freq,
                         i2c_master_dev_handle_t *dev_handle);

#endif
