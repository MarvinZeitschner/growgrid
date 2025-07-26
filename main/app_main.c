#include "bmp280.h"
#include "board.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "hal/i2c_types.h"
#include "i2c_bus.h"
#include "mqtt.h"
#include "rgb_led.h"
#include "soc/gpio_num.h"
#include "soil_sensor.h"
#include "tsl2561.h"
#include "wifi.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static const char *TAG = "APP_MAIN";

i2c_bus_handle_t i2c_bus;
i2c_bus_device_handle_t tsl2561_dev_handle;

tsl2561_handle_t tsl2561;
bmp280_handle_t bmp280;
soil_sensor_handle_t soil_sensor;

void read_light(void *pvParameter) {
  while (1) {
    uint32_t lux = 0;
    esp_err_t err = tsl2561_read_lux(tsl2561, &lux);
    if (err == ESP_OK) {
      printf("Ambient light: %" PRIu32 " lux\n", lux);
    } else {
      ESP_LOGE(TAG, "Failed to read TSL2561: %s\n", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void read_temperature(void *pvParameter) {
  while (1) {
    float temperature = 0.0;
    esp_err_t err = bmp280_read_temperature(bmp280, &temperature);
    if (err == ESP_OK) {
      printf("Temperature: %f deg\n", temperature);
    } else {
      ESP_LOGE(TAG, "Failed to read bmp280 temp: %s\n", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void read_soil_moisture(void *pvParameter) {
  while (1) {
    int percent = 0, raw = 0;
    soil_sensor_read_raw(soil_sensor, &raw);
    soil_sensor_read_percent(soil_sensor, &percent);
    printf("Soil sensor raw value: %d\n", raw);
    printf("Soil sensor mapped value: %d\n", percent);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void i2c_configure() {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_SDA_PIN,
      .scl_io_num = I2C_SCL_PIN,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_FREQ_HZ,
  };
  i2c_bus = i2c_bus_create(I2C_PORT, &conf);
}

esp_err_t tsl2561_configure() {
  tsl2561 = tsl2561_create(i2c_bus, TSL2561_I2C_ADDR);
  return tsl2561_default_init(tsl2561);
}

esp_err_t bmp280_configure() {
  bmp280 = bmp280_create(i2c_bus, BMP280_I2C_ADDRESS_DEFAULT);
  return bmp280_default_init(bmp280);
}

void soil_sensor_configure() {
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  adc_oneshot_chan_cfg_t ch_config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  soil_sensor_config_t s_conf = {
      .adc_pin = GPIO_NUM_0,
      .init_config = init_config,
      .channel_config = ch_config,
  };

  soil_sensor = soil_sensor_create(s_conf);

  soil_sensor_set_calibration(soil_sensor, SOIL_OUT_MAX, SOIL_OUT_MIN);
}

void app_main(void) {
  ESP_ERROR_CHECK(wifi_init_sta());
  mqtt_app_start();

  i2c_configure();

  ESP_ERROR_CHECK(rgb_led__default_init());
  ESP_ERROR_CHECK(rgb_led_set_color(0, 255, 0));

  soil_sensor_configure();

  ESP_ERROR_CHECK(tsl2561_configure());
  ESP_ERROR_CHECK(bmp280_configure());

  vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for sensors to stabilize
  ESP_ERROR_CHECK(rgb_led_clear());

  ESP_ERROR_CHECK(xTaskCreate(&read_light, "read_light", 2048, NULL, 4, NULL) == pdTRUE ? ESP_OK : ESP_FAIL);
  ESP_ERROR_CHECK(xTaskCreate(&read_temperature, "read_temperature", 2048, NULL, 4, NULL) == pdTRUE ? ESP_OK : ESP_FAIL);
  ESP_ERROR_CHECK(xTaskCreate(&read_soil_moisture, "read_soil_moisture", 2048, NULL, 5, NULL) == pdTRUE ? ESP_OK : ESP_FAIL);
}
