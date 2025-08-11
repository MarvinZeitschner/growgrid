#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "soc/gpio_num.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SOIL_SAMPLING_X4,
  SOIL_SAMPLING_X8,
  SOIL_SAMPLING_X16,
} soil_sensor_sampling;

typedef struct {
  adc_oneshot_unit_init_cfg_t init_config;
  adc_oneshot_chan_cfg_t channel_config;
  gpio_num_t adc_pin;
  soil_sensor_sampling sampling;
} soil_sensor_config_t;

typedef struct {
  uint16_t min;
  uint16_t max;
  soil_sensor_config_t config;
  adc_oneshot_unit_handle_t adc_oneshot_handle;
} soil_sensor_dev_t;

typedef void *soil_sensor_handle_t;

/**
 * @brief create soil sensor
 *
 * param[in] config for the sensor
 *
 * @return
 *     - soil_sensor_handle_t
 */
soil_sensor_handle_t soil_sensor_create(soil_sensor_config_t const config);

/**
 * @brief read raw analog value
 *
 * param[in] sensor handle
 * param[out] raw analog value
 *
 * @return
 *     - esp_err_t
 */
esp_err_t soil_sensor_read_raw(soil_sensor_handle_t sensor, int *raw);

/**
 * @brief read analog value as percentage
 *
 * param[in] sensor handle
 * param[out] percentage value
 *
 * @return
 *     - esp_err_t
 */
esp_err_t soil_sensor_read_percent(soil_sensor_handle_t sensor, int *percent);

/**
 * @brief Set the calibration values of the sensor
 *
 * param[in] dry (maximum) value
 * param[in] wet (minimum) value
 *
 * @return
 *     - esp_err_t
 */
void soil_sensor_set_calibration(soil_sensor_handle_t sensor, int dry, int wet);

/**
 * @brief   delete soil handle_t
 *
 * @param  point to object handle of soil sensor
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t soil_sensor_delete(soil_sensor_handle_t *sensor);

#ifdef __cplusplus
}
#endif
