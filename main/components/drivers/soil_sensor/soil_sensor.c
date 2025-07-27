#include "soil_sensor.h"
#include "board.h"
#include "esp_err.h"
#include "esp_log.h"
#include "map_value.h"
#include <stdlib.h>

static const char *TAG = "SOIL";

soil_sensor_handle_t soil_sensor_create(soil_sensor_config_t const config) {
  soil_sensor_dev_t *sens =
      (soil_sensor_dev_t *)calloc(1, sizeof(soil_sensor_dev_t));

  sens->config = config;
  sens->min = 0;
  // TODO: derive standard from config
  sens->max = 4095;

  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  adc_oneshot_chan_cfg_t ch_conf = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  ESP_ERROR_CHECK(
      adc_oneshot_new_unit(&init_config1, &sens->adc_oneshot_handle));

  ESP_ERROR_CHECK(adc_oneshot_config_channel(sens->adc_oneshot_handle,
                                             SOIL_ADC_CHANNEL, &ch_conf));

  ESP_LOGI(TAG, "soil sensor initiated");

  return (soil_sensor_handle_t)sens;
}

esp_err_t soil_sensor_read_raw(soil_sensor_handle_t sensor, int *raw) {
  soil_sensor_dev_t *sens = (soil_sensor_dev_t *)sensor;
  return adc_oneshot_read(sens->adc_oneshot_handle, SOIL_ADC_CHANNEL, raw);
}

esp_err_t soil_sensor_read_percent(soil_sensor_handle_t sensor, int *percent) {
  soil_sensor_dev_t *sens = (soil_sensor_dev_t *)sensor;
  int raw = 0;
  esp_err_t err = soil_sensor_read_raw(sensor, &raw);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error reading raw value");
    return err;
  }
  *percent = map_value(raw, sens->min, sens->max, 100, 0);
  if (*percent > 100) {
    *percent = 100;
  }

  return ESP_OK;
}

void soil_sensor_set_calibration(soil_sensor_handle_t sensor, int dry,
                                 int wet) {
  soil_sensor_dev_t *sens = (soil_sensor_dev_t *)sensor;
  sens->max = dry;
  sens->min = wet;
}

esp_err_t soil_sensor_delete(soil_sensor_handle_t *sensor) {
  if (*sensor == NULL) {
    return ESP_OK;
  }
  soil_sensor_dev_t *sens = (soil_sensor_dev_t *)(*sensor);
  free(sens);
  *sensor = NULL;
  return ESP_OK;
}
