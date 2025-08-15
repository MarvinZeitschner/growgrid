#include "pump_control_service.h"
#include "app_config.h"
#include "board.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"

static const char *TAG = "PUMP_CONTROL";

static void pump_control_service() {
  gpio_config_t io_conf = {.pin_bit_mask = (1ULL << PUMP_RELAY_GPIO),
                           .mode = GPIO_MODE_OUTPUT,
                           .pull_up_en = GPIO_PULLUP_DISABLE,
                           .pull_down_en = GPIO_PULLDOWN_DISABLE,
                           .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  while (1) {
    // ESP_LOGI(TAG, "ZERO");
    // gpio_set_level(PUMP_RELAY_GPIO, 0);
    //
    // vTaskDelay(pdMS_TO_TICKS(5000));
    //
    // ESP_LOGI(TAG, "ONE");
    // gpio_set_level(PUMP_RELAY_GPIO, 1);

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

esp_err_t pump_control_service_start() {
  xTaskCreate(&pump_control_service, "pump_control", TASK_STACK_PUMP_CONTROL,
              NULL, TASK_PRIO_PUMP_CONTROLER_SERIVCE, NULL);

  ESP_LOGI(TAG, "Pump control service started.");
  return ESP_OK;
}
