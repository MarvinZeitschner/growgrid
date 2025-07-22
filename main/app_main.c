#include "board.h"
#include "components/drivers/include/i2c.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "rgb_led.h"
#include <stdio.h>

i2c_master_bus_handle_t i2c_mst_bus;
i2c_master_dev_handle_t tsl2561_dev_handle;

void hello_task(void *pvParameter) {
  while (1) {
    printf("Hello from FreeRTOS task!\n");
    rgb_led_set_color(0, 10, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    rgb_led_clear();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  ESP_ERROR_CHECK(i2c_init(I2C_NUM_0, SDA_GPIO, SCL_GPIO, &i2c_mst_bus));

  ESP_ERROR_CHECK(
      i2c_add_device(i2c_mst_bus, 0x39, 100000, &tsl2561_dev_handle));

  rgb_led_init();

  xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
}
