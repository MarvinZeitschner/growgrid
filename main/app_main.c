#include "board.h"
#include "components/drivers/include/i2c.h"
#include "components/tsl2561/include/tsl2561.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "rgb_led.h"
#include <stdint.h>
#include <stdio.h>

static const char *TAG = "APP_MAIN";

i2c_master_bus_handle_t i2c_mst_bus;
i2c_master_dev_handle_t tsl2561_dev_handle;

tsl2561_t tsl2561_instance;

void read_tsl2561_task(void *pvParameter) {
  uint16_t ch0 = 0;
  while (1) {
    esp_err_t err = tsl2561_read_ch0(&tsl2561_instance, &ch0, 1000);
    if (err != ESP_OK)
      ESP_LOGE(TAG, "error reading tsl2561 sensor");

    printf("TSL2561 Channel 0: %u\n", ch0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  ESP_ERROR_CHECK(i2c_init(I2C_NUM_0, SDA_GPIO, SCL_GPIO, &i2c_mst_bus));

  ESP_ERROR_CHECK(
      i2c_add_device(i2c_mst_bus, 0x39, 100000, &tsl2561_dev_handle));

  ESP_ERROR_CHECK(tsl2561_init(&tsl2561_instance, tsl2561_dev_handle, 1000));

  rgb_led_init();

  xTaskCreate(&read_tsl2561_task, "read_tsl2561_task", 2048, NULL, 5, NULL);
}
