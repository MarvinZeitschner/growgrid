#include "board.h"
#include "components/bmp280/include/bmp280.h"
#include "components/tsl2561/include/tsl2561.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "i2c_bus.h"
#include "rgb_led.h"
#include <stdint.h>
#include <stdio.h>

static const char *TAG = "APP_MAIN";

i2c_bus_handle_t i2c_bus;
i2c_bus_device_handle_t tsl2561_dev_handle;

tsl2561_handle_t tsl2561;
bmp280_handle_t bmp280;

void read_sensors_task(void *pvParameter) {
  while (1) {
    uint32_t lux = 0;
    esp_err_t err = tsl2561_read_lux(&tsl2561, &lux, 1000);
    if (err == ESP_OK) {
      printf("Ambient light: %" PRIu32 " lux\n", lux);
    } else {
      ESP_LOGE(TAG, "Failed to read TSL2561: %s", esp_err_to_name(err));
    }

    float temperature = 0.0;
    err = bmp280_read_temperature(bmp280, &temperature);
    if (err == ESP_OK) {
      printf("Temperature: %f deg\n", temperature);
    } else {
      ESP_LOGE(TAG, "Failed to read bmp280 temp: %s\n", esp_err_to_name(err));
    }

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

  tsl2561_dev_handle =
      i2c_bus_device_create(i2c_bus, TSL2561_I2C_ADDR, I2C_FREQ_HZ);
}

void tsl2561_configure() {
  tsl2561_config_t tsl2561_conf = {
      .gain = TSL2561_GAIN_1x,
      .integration_time = TSL2561_INTEGRATION_402,
  };
  ESP_ERROR_CHECK(
      tsl2561_init(&tsl2561, tsl2561_dev_handle, &tsl2561_conf, 1000));
}

void bmp280_configure() {
  bmp280 = bmp280_create(i2c_bus, BMP280_I2C_ADDRESS_DEFAULT);
  esp_err_t err = bmp280_default_init(bmp280);
  ESP_LOGI(TAG, "bmp280_default_init: %s", esp_err_to_name(err));
}

void app_main(void) {
  i2c_configure();
  tsl2561_configure();
  bmp280_configure();
  rgb_led_init();

  // Wait for sensors to become stable (esp. bmp280)
  vTaskDelay(pdMS_TO_TICKS(100));

  xTaskCreate(&read_sensors_task, "read_sensors_task", 4096, NULL, 5, NULL);
}
