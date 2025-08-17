#include "sensor_tasks.h"
#include "app_config.h"
#include "esp_log.h"
#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "growgrid_types.h"
#include "hal_sensors.h"

static const char *TAG = "SENSOR_TASKS";

static void fast_sensor_task(void *pvParameters) {
  TickType_t last_wake_time = xTaskGetTickCount();
  ESP_LOGI(TAG, "Fast sensor task started");

  while (1) {
    event_t event;
    temp_humidity_data_t temp_hum_data;
    light_data_t light_data;

    if (hal_sensors_read_temp_humidity(&temp_hum_data) == ESP_OK) {
      event.type = EVENT_TYPE_SENSOR_DATA;
      event.data.sensor_data.type = SENSOR_DATA_TYPE_TEMP_HUMIDITY;
      event.data.sensor_data.payload.temp_humidity = temp_hum_data;
      event_bus_post(&event, pdMS_TO_TICKS(EVENT_BUS_POST_TIMEOUT_MS));
    } else {
      ESP_LOGE(TAG, "Failed to read temperature/humidity");
    }

    if (hal_sensors_read_light(&light_data) == ESP_OK) {
      event.type = EVENT_TYPE_SENSOR_DATA;
      event.data.sensor_data.type = SENSOR_DATA_TYPE_LIGHT;
      event.data.sensor_data.payload.light = light_data;
      event_bus_post(&event, pdMS_TO_TICKS(EVENT_BUS_POST_TIMEOUT_MS));
    } else {
      ESP_LOGE(TAG, "Failed to read light");
    }

    vTaskDelayUntil(&last_wake_time,
                    pdMS_TO_TICKS(FAST_SENSOR_READ_INTERVAL_MS));
  }
}

static void slow_sensor_task(void *pvParameters) {
  TickType_t last_wake_time = xTaskGetTickCount();
  ESP_LOGI(TAG, "Slow sensor task started");

  while (1) {
    event_t event;
    soil_moisture_data_t soil_data;

    if (hal_sensors_read_soil_moisture(&soil_data) == ESP_OK) {
      event.type = EVENT_TYPE_SENSOR_DATA;
      event.data.sensor_data.type = SENSOR_DATA_TYPE_SOIL_MOISTURE;
      event.data.sensor_data.payload.soil_moisture = soil_data;
      event_bus_post(&event, pdMS_TO_TICKS(EVENT_BUS_POST_TIMEOUT_MS));
    } else {
      ESP_LOGE(TAG, "Failed to read soil moisture");
    }

    vTaskDelayUntil(&last_wake_time,
                    pdMS_TO_TICKS(SLOW_SENSOR_READ_INTERVAL_MS));
  }
}

esp_err_t app_sensor_tasks_start(void) {
  if (xTaskCreate(fast_sensor_task, "fast_sensor_task", TASK_STACK_FAST_SENSOR,
                  NULL, TASK_PRIO_FAST_SENSOR, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create fast sensor task");
    return ESP_FAIL;
  }
  if (xTaskCreate(slow_sensor_task, "slow_sensor_task", TASK_STACK_SLOW_SENSOR,
                  NULL, TASK_PRIO_SLOW_SENSOR, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create slow sensor task");
    return ESP_FAIL;
  }
  return ESP_OK;
}
