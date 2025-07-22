#ifndef TSL2561_H
#define TSL2561_H

#include "driver/i2c_types.h"
#include "esp_err.h"
#include <stdint.h>

#define TSL2561_COMMAND 0x80
#define TSL2561_REG_CONTROL 0x00
#define TSL2561_DATA0LOW 0x0C

typedef struct {
  i2c_master_dev_handle_t dev_handle;
} tsl2561_t;

/**
 * @brief Initializes the tsl2561_t struct and turns on the sensor
 *
 * @param sensor: Struct to be initialized
 * @param dev_handle: Handle to i2c device that gets put in tsl2561_t
 *
 * @returns
 *     - ESP_OK: I2C master transmit success
 *     - ESP_ERR_INVALID_ARG: I2C master transmit parameter invalid.
 *     - ESP_ERR_TIMEOUT: Operation timeout(larger than xfer_timeout_ms) because
 * the bus is busy or hardware crash.
 */
esp_err_t tsl2561_init(tsl2561_t *sensor, i2c_master_dev_handle_t dev_handle,
                       int timeout);

/**
 * @brief Reads the values from channel0
 *
 * @param sensor: tsl2561 instance
 * @param ch0: Variable where sensor value gets saved
 * @param timeout: Setable timeout
 *
 * @returns
 *     - ESP_OK: I2C master transmit success
 *     - ESP_ERR_INVALID_ARG: I2C master transmit parameter invalid.
 *     - ESP_ERR_TIMEOUT: Operation timeout(larger than xfer_timeout_ms) because
 * the bus is busy or hardware crash.
 */
esp_err_t tsl2561_read_ch0(tsl2561_t *sensor, uint16_t *ch0, int timeout);

/**
 * @brief Powers off tsl2561 (DOESN'T invalide sensor)
 *
 * @param sensor: tsl2561 instance
 * @param timeout: Setable timeout
 *
 * @returns
 *     - ESP_OK: I2C master transmit success
 *     - ESP_ERR_INVALID_ARG: I2C master transmit parameter invalid.
 *     - ESP_ERR_TIMEOUT: Operation timeout(larger than xfer_timeout_ms) because
 * the bus is busy or hardware crash.
 */

esp_err_t tsl2561_power_off(tsl2561_t *sensor, int timeout);

#endif // !TSL2561_H
