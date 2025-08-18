#pragma once
#include "esp_err.h"
#include "growgrid_types.h"

/**
 * @brief Initializes all sensors.
 * @return ESP_OK on success.
 */
esp_err_t hal_sensors_init(void);

/**
 * @brief Reads temperature and humidity.
 * @param[out] data Pointer to a struct to store the data.
 * @return ESP_OK on success.
 */
esp_err_t hal_sensors_read_temp_humidity(temp_humidity_data_t *data);

/**
 * @brief Reads light level.
 * @param[out] data Pointer to a struct to store the data.
 * @return ESP_OK on success.
 */
esp_err_t hal_sensors_read_light(light_data_t *data);

/**
 * @brief Reads soil moisture.
 * @param[out] data Pointer to a struct to store the data.
 * @return ESP_OK on success.
 */
esp_err_t hal_sensors_read_soil_moisture(soil_moisture_data_t *data);
