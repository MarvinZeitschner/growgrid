#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "rgb_led.h"
#include <stdio.h>

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
  rgb_led_init();
  xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
}
