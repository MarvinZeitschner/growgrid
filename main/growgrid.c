#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <stdio.h>

void hello_task(void *pvParameter) {
  while (1) {
    printf("Hello from FreeRTOS task!\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
}
