/**
 * @file tsl2561.h
 * @brief Driver for the TSL2561 light sensor (I2C, fixed Address 0x39)
 */

#pragma once

#include "driver/i2c_types.h"
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TSL2561_I2C_ADDR 0x39 ///< Default I2C address (ADDR pin floating)

#define TSL2561_CMD 0x80           ///< Command bit for register access
#define TSL2561_CMD_WORD 0xA0      ///< Command + word bit for 16-bit read
#define TSL2561_REG_CONTROL 0x00   ///< Control register address
#define TSL2561_REG_TIMING 0x01    ///< Timing register address
#define TSL2561_REG_ID 0x0A        ///< ID register address
#define TSL2561_REG_DATA0LOW 0x0C  ///< Channel 0 low byte
#define TSL2561_REG_DATA0HIGH 0x0D ///< Channel 0 high byte
#define TSL2561_REG_DATA1LOW 0x0E  ///< Channel 1 low byte
#define TSL2561_REG_DATA1HIGH 0x0F ///< Channel 1 high byte

#define TSL2561_CMD_POWER_ON 0x03  ///< Power on value for CONTROL register
#define TSL2561_CMD_POWER_OFF 0x00 ///< Power off value for CONTROL register

/**
 * @brief Integration time options for the TSL2561
 *
 * Determines how long the sensor integrates time before reporting a value
 */
typedef enum {
  TSL2561_INTEGRATION_13 = 0x00,
  TSL2561_INTEGRATION_101 = 0x01,
  TSL2561_INTEGRATION_402 = 0x02,
} tsl2561_integration_time_t;

/**
 * @brief Gain options for the TSL2561
 *
 * Selects the sensor's gain (sensitivity)
 */
typedef enum {
  TSL2561_GAIN_1x = 0x00,
  TSL2561_GAIN_16x = 0x10,
} tsl2561_gain_t;

/**
 * @brief Configuration struct for the TSL2561
 *
 *  Used to set integration time and gain
 */
typedef struct {
  tsl2561_integration_time_t integration_time;
  tsl2561_gain_t gain;
} tsl2561_config_t;

/**
 * @brief Device handle
 *
 * Stores sensor configuration and I2C device handle
 */
typedef struct {
  tsl2561_integration_time_t integration_time;
  tsl2561_gain_t gain;
  i2c_master_dev_handle_t dev_handle;
} tsl2561_t;

/**
 * @brief Initialize the TSL2561 sensor
 *
 * Powers on the sensor and sets integration time and gain.
 *
 * @param[out] sensor      Pointer to TSL2561 device handle to initialize
 * @param[in]  dev_handle  ESP-IDF I2C device handle (from
 * i2c_master_bus_add_device)
 * @param[in]  config      Pointer to configuration struct (integration time,
 * gain)
 * @param[in]  timeout     I2C operation timeout in milliseconds
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_init(tsl2561_t *sensor, i2c_master_dev_handle_t dev_handle,
                       const tsl2561_config_t *config, int timeout);

/**
 * @brief Power off the TSL2561 sensor
 *
 * Puts the sensor into a low-power state.
 *
 * @param[in] sensor   Pointer to initialized TSL2561 device handle
 * @param[in] timeout  I2C operation timeout in milliseconds
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_power_off(tsl2561_t *sensor, int timeout);

/**
 * @brief Set integration time and gain
 *
 * Updates the sensor's integration time and gain settings.
 *
 * @param[in,out] sensor  Pointer to initialized TSL2561 device handle
 * @param[in]     config  Pointer to new configuration struct
 * @param[in]     timeout I2C operation timeout in milliseconds
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_set_config(tsl2561_t *sensor, const tsl2561_config_t *config,
                             int timeout);

/**
 * @brief Read raw ADC values from both channels
 *
 * Reads the 16-bit values from channel 0 (visible + IR) and channel 1 (IR
 * only).
 *
 * @param[in]  sensor   Pointer to initialized TSL2561 device handle
 * @param[out] ch0      Pointer to store channel 0 value
 * @param[out] ch1      Pointer to store channel 1 value
 * @param[in]  timeout  I2C operation timeout in milliseconds
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_read_channels(tsl2561_t *sensor, uint16_t *ch0, uint16_t *ch1,
                                int timeout);

#ifdef __cplusplus
}
#endif
