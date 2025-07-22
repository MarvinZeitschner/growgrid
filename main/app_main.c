#include "board.h"
#include "components/drivers/include/i2c.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "rgb_led.h"
#include <stdint.h>
#include <stdio.h>

i2c_master_bus_handle_t i2c_mst_bus;
i2c_master_dev_handle_t tsl2561_dev_handle;

void hello_task(void *pvParameter) {
  while (1) {
    uint8_t reg = 0x8C;
    uint8_t data[2] = {0};
    ESP_ERROR_CHECK(i2c_master_transmit_receive(tsl2561_dev_handle, &reg, 1,
                                                data, 2, 1000));
    uint16_t channel0 = (data[1] << 8) | data[0];
    printf("TSL2561 Channel 0: %u\n", channel0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  ESP_ERROR_CHECK(i2c_init(I2C_NUM_0, SDA_GPIO, SCL_GPIO, &i2c_mst_bus));

  ESP_ERROR_CHECK(
      i2c_add_device(i2c_mst_bus, 0x39, 100000, &tsl2561_dev_handle));

  uint8_t power_cmd[2] = {0x80, 0x03};
  ESP_ERROR_CHECK(i2c_master_transmit(tsl2561_dev_handle, power_cmd, 2, 1000));

  rgb_led_init();

  xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
}
