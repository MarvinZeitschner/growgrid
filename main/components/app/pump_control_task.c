#include "pump_control_task.h"
#include "app_config.h"
#include "esp_log.h"
#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "hal_pump.h"
// #include "pump_logic.h"

static const char *TAG = "PUMP_CONTROL_TASK";

static void pump_control_task(void *pvParameters) {
  QueueHandle_t event_queue = event_bus_subscribe();
  if (event_queue == NULL) {
    ESP_LOGE(TAG, "Failed to subscribe to event bus");
    vTaskDelete(NULL);
  }
  ESP_LOGI(TAG, "Pump control task started");

  while (1) {
    event_t event;
    if (xQueueReceive(event_queue, &event, portMAX_DELAY) == pdTRUE) {
      if (event.type == EVENT_TYPE_SENSOR_DATA &&
          event.data.sensor_data.type == SENSOR_DATA_TYPE_SOIL_MOISTURE) {
        int moisture = event.data.sensor_data.payload.soil_moisture.percent;
        ESP_LOGI(TAG, "Received soil moisture: %d%%", moisture);
        // if (pump_logic_should_start(moisture)) {
        //   ESP_LOGI(TAG, "Moisture is low, turning pump ON");
        //   hal_pump_on();
        // } else {
        //   ESP_LOGI(TAG, "Moisture is sufficient, turning pump OFF");
        //   hal_pump_off();
        // }
      }
    }
  }
}

esp_err_t app_pump_control_task_start(void) {
  if (xTaskCreate(pump_control_task, "pump_control_task",
                  TASK_STACK_PUMP_CONTROL, NULL, TASK_PRIO_PUMP_CONTROL,
                  NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create pump control task");
    return ESP_FAIL;
  }
  return ESP_OK;
}
