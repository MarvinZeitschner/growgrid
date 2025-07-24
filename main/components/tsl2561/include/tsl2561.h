/**
 * @file tsl2561.h
 * @brief Driver for the TSL2561 light sensor (I2C, fixed Address 0x39)
 */

#pragma once

#include "esp_err.h"
#include "i2c_bus.h"
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

#define TSL2561_LUX_SCALE 14  // scale by 2^14
#define TSL2561_RATIO_SCALE 9 // scale ratio by 2^9
// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
//  Integration time scaling factors
// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
#define TSL2561_CH_SCALE 10          // scale channel values by 2^10
#define TSL2561_CHSCALE_TINT0 0x7517 // 322/11 * 2^CH_SCALE
#define TSL2561_CHSCALE_TINT1 0x0fe7 // 322/81 * 2^CH_SCALE

// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
//  T Package coefficients
// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
#define TSL2561_K1T 0x0040 // 0.125 * 2^RATIO_SCALE
#define TSL2561_B1T 0x01f2 // 0.0304 * 2^LUX_SCALE
#define TSL2561_M1T 0x01be // 0.0272 * 2^LUX_SCALE

#define TSL2561_K2T 0x0080 // 0.250 * 2^RATIO_SCALE
#define TSL2561_B2T 0x0214 // 0.0325 * 2^LUX_SCALE
#define TSL2561_M2T 0x02d1 // 0.0440 * 2^LUX_SCALE

#define TSL2561_K3T 0x00c0 // 0.375 * 2^RATIO_SCALE
#define TSL2561_B3T 0x023f // 0.0351 * 2^LUX_SCALE
#define TSL2561_M3T 0x037b // 0.0544 * 2^LUX_SCALE

#define TSL2561_K4T 0x0100 // 0.50 * 2^RATIO_SCALE
#define TSL2561_B4T 0x0270 // 0.0381 * 2^LUX_SCALE
#define TSL2561_M4T 0x03fe // 0.0624 * 2^LUX_SCALE

#define TSL2561_K5T 0x0138 // 0.61 * 2^RATIO_SCALE
#define TSL2561_B5T 0x016f // 0.0224 * 2^LUX_SCALE
#define TSL2561_M5T 0x01fc // 0.0310 * 2^LUX_SCALE

#define TSL2561_K6T 0x019a // 0.80 * 2^RATIO_SCALE
#define TSL2561_B6T 0x00d2 // 0.0128 * 2^LUX_SCALE
#define TSL2561_M6T 0x00fb // 0.0153 * 2^LUX_SCALE

#define TSL2561_K7T 0x029a // 1.3 * 2^RATIO_SCALE
#define TSL2561_B7T 0x0018 // 0.00146 * 2^LUX_SCALE
#define TSL2561_M7T 0x0012 // 0.00112 * 2^LUX_SCALE

#define TSL2561_K8T 0x029a // 1.3 * 2^RATIO_SCALE
#define TSL2561_B8T 0x0000 // 0.000 * 2^LUX_SCALE
#define TSL2561_M8T 0x0000 // 0.000 * 2^LUX_SCALE

// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
//  CS package coefficients
// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
#define TSL2561_K1C 0x0043 // 0.130 * 2^RATIO_SCALE
#define TSL2561_B1C 0x0204 // 0.0315 * 2^LUX_SCALE
#define TSL2561_M1C 0x01ad // 0.0262 * 2^LUX_SCALE

#define TSL2561_K2C 0x0085 // 0.260 * 2^RATIO_SCALE
#define TSL2561_B2C 0x0228 // 0.0337 * 2^LUX_SCALE
#define TSL2561_M2C 0x02c1 // 0.0430 * 2^LUX_SCALE

#define TSL2561_K3C 0x00c8 // 0.390 * 2^RATIO_SCALE
#define TSL2561_B3C 0x0253 // 0.0363 * 2^LUX_SCALE
#define TSL2561_M3C 0x0363 // 0.0529 * 2^LUX_SCALE

#define TSL2561_K4C 0x010a // 0.520 * 2^RATIO_SCALE
#define TSL2561_B4C 0x0282 // 0.0392 * 2^LUX_SCALE
#define TSL2561_M4C 0x03df // 0.0605 * 2^LUX_SCALE

#define TSL2561_K5C 0x014d // 0.65 * 2^RATIO_SCALE
#define TSL2561_B5C 0x0177 // 0.0229 * 2^LUX_SCALE
#define TSL2561_M5C 0x01dd // 0.0291 * 2^LUX_SCALE

#define TSL2561_K6C 0x019a // 0.80 * 2^RATIO_SCALE
#define TSL2561_B6C 0x0101 // 0.0157 * 2^LUX_SCALE
#define TSL2561_M6C 0x0127 // 0.0180 * 2^LUX_SCALE

#define TSL2561_K7C 0x029a // 1.3 * 2^RATIO_SCALE
#define TSL2561_B7C 0x0037 // 0.00338 * 2^LUX_SCALE
#define TSL2561_M7C 0x002b // 0.00260 * 2^LUX_SCALE

#define TSL2561_K8C 0x029a // 1.3 * 2^RATIO_SCALE
#define TSL2561_B8C 0x0000 // 0.000 * 2^LUX_SCALE
#define TSL2561_M8C 0x0000 // 0.000 * 2^LUX_SCALE
// −−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−

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
  i2c_bus_device_handle_t i2c_dev;
  uint8_t dev_addr;
} tsl2561_dev_t;

typedef void *tsl2561_handle_t; /*handle of tsl2561*/

/**
 * @brief   Create bmp280 handle_t
 *
 * @param  object handle of I2C
 * @param  device address
 *
 * @return
 *     - bmp280_handle_t
 */
tsl2561_handle_t tsl2561_create(i2c_bus_handle_t bus, uint8_t dev_addr);

/**
 * @brief   delete bmp280 handle_t
 *
 * @param  point to object handle of bmp280
 * @param  whether delete i2c bus
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tsl2561_delete(tsl2561_handle_t *sensor);

/**
 * @brief Initialize the TSL2561 sensor
 *
 * Powers on the sensor and sets integration time and gain.
 *
 * @param[out] sensor      Pointer to TSL2561 device handle to initialize
 * @param[in]  dev_handle  ESP-IDF I2C device handle
 * @param[in]  config      Pointer to configuration struct (integration time,
 * gain)
 *
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_default_init(tsl2561_handle_t sensor);

/**
 * @brief Power on the TSL2561 sensor
 *
 * Puts the sensor into a high-power state.
 *
 * @param[in] sensor   Pointer to initialized TSL2561 device handle
 *
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_power_on(tsl2561_handle_t sensor);

/**
 * @brief Power off the TSL2561 sensor
 *
 * Puts the sensor into a low-power state.
 *
 * @param[in] sensor   Pointer to initialized TSL2561 device handle
 *
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_power_off(tsl2561_handle_t sensor);

/**
 * @brief Set integration time and gain
 *
 * Updates the sensor's integration time and gain settings.
 *
 * @param[in,out] sensor  Pointer to initialized TSL2561 device handle
 * @param[in]     config  Pointer to new configuration struct
 *
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_set_config(tsl2561_handle_t sensor,
                             const tsl2561_config_t config);

/**
 * @brief Read raw ADC values from both channels
 *
 * Reads the 16-bit values from channel 0 (visible + IR) and channel 1 (IR
 * only).
 *
 * @param[in]  sensor   Pointer to initialized TSL2561 device handle
 * @param[out] ch0      Pointer to store channel 0 value
 * @param[out] ch1      Pointer to store channel 1 value
 *
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_read_channels(tsl2561_handle_t sensor, uint16_t *ch0,
                                uint16_t *ch1);

/**
 * @brief Read light level in lux
 *
 * Reads both channels and calculates light in lux.
 *
 * @param[in]  sensor   Pointer to initialized TSL2561 device handle
 * @param[out] lux      Pointer to store calculated lux value
 *
 * @return ESP_OK on success, or an error code from the I2C driver
 */
esp_err_t tsl2561_read_lux(tsl2561_handle_t sensor, uint32_t *lux);

#ifdef __cplusplus
}
#endif
