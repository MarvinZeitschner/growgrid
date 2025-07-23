#include "board.h"
#include "components/tsl2561/include/tsl2561.h"
#include "driver/i2c_master.h"
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

tsl2561_t tsl2561;

void read_tsl2561_task(void *pvParameter) {
  uint16_t ch0 = 0, ch1 = 0;
  while (1) {
    esp_err_t err = tsl2561_read_channels(&tsl2561, &ch0, &ch1, 1000);
    if (err == ESP_OK) {
      printf("TSL2561: CH0 (Visible+IR) = %u, CH1 (IR only) = %u\n", ch0, ch1);
    } else {
      ESP_LOGE(TAG, "Failed to read TSL2561: %s", esp_err_to_name(err));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  i2c_master_bus_config_t mst_bus_conf = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_PORT,
      .scl_io_num = I2C_SCL_PIN,
      .sda_io_num = I2C_SDA_PIN,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&mst_bus_conf, &i2c_mst_bus));

  i2c_device_config_t tsl2561_dev_conf = {
      .device_address = TSL2561_I2C_ADDR,
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .scl_speed_hz = I2C_FREQ_HZ,
  };
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_mst_bus, &tsl2561_dev_conf,
                                            &tsl2561_dev_handle));

  tsl2561_config_t tsl2561_conf = {
      .gain = TSL2561_GAIN_1x,
      .integration_time = TSL2561_INTEGRATION_402,
  };
  ESP_ERROR_CHECK(
      tsl2561_init(&tsl2561, tsl2561_dev_handle, &tsl2561_conf, 1000));

  rgb_led_init();

  xTaskCreate(&read_tsl2561_task, "read_tsl2561_task", 2048, NULL, 5, NULL);
}
