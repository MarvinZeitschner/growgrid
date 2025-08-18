#include "sensor_tasks.h"
#include "app_config.h"
#include "esp_log.h"
#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "growgrid_types.h"
#include "hal_sensors.h"
#include <sys/time.h>

static const char *TAG = "SENSOR_TASKS";

static void temp_humidity_sensor_task(void *pvParameters) {
  TickType_t last_wake_time = xTaskGetTickCount();
  ESP_LOGI(TAG, "Temperature & Humidity sensor task started");

  while (1) {
    event_t event;
    temp_humidity_data_t temp_hum_data;
    struct timeval tv_now;

    if (hal_sensors_read_temp_humidity(&temp_hum_data) == ESP_OK) {
      gettimeofday(&tv_now, NULL);
      event.type = EVENT_TYPE_SENSOR_DATA;
      event.data.sensor_data.type = SENSOR_DATA_TYPE_TEMP_HUMIDITY;
      event.data.sensor_data.payload.temp_humidity = temp_hum_data;
      event.data.sensor_data.timestamp_us =
          (uint64_t)tv_now.tv_sec * 1000000L + (uint64_t)tv_now.tv_usec;
      event_bus_post(&event, pdMS_TO_TICKS(EVENT_BUS_POST_TIMEOUT_MS));
    } else {
      ESP_LOGE(TAG, "Failed to read temperature/humidity");
    }
    vTaskDelayUntil(&last_wake_time,
                    pdMS_TO_TICKS(TEMP_SENSOR_READ_INTERVAL_MS));
  }
}

static void light_sensor_task(void *pvParameters) {
  TickType_t last_wake_time = xTaskGetTickCount();
  ESP_LOGI(TAG, "Light sensor task started");

  while (1) {
    event_t event;
    light_data_t light_data;
    struct timeval tv_now;

    if (hal_sensors_read_light(&light_data) == ESP_OK) {
      gettimeofday(&tv_now, NULL);
      event.type = EVENT_TYPE_SENSOR_DATA;
      event.data.sensor_data.type = SENSOR_DATA_TYPE_LIGHT;
      event.data.sensor_data.payload.light = light_data;
      event.data.sensor_data.timestamp_us =
          (uint64_t)tv_now.tv_sec * 1000000L + (uint64_t)tv_now.tv_usec;
      event_bus_post(&event, pdMS_TO_TICKS(EVENT_BUS_POST_TIMEOUT_MS));
    } else {
      ESP_LOGE(TAG, "Failed to read light");
    }
    vTaskDelayUntil(&last_wake_time,
                    pdMS_TO_TICKS(LIGHT_SENSOR_READ_INTERVAL_MS));
  }
}

static void soil_moisture_sensor_task(void *pvParameters) {
  TickType_t last_wake_time = xTaskGetTickCount();
  ESP_LOGI(TAG, "Soil Moisture sensor task started");

  while (1) {
    event_t event;
    soil_moisture_data_t soil_data;
    struct timeval tv_now;

    if (hal_sensors_read_soil_moisture(&soil_data) == ESP_OK) {
      gettimeofday(&tv_now, NULL);
      event.type = EVENT_TYPE_SENSOR_DATA;
      event.data.sensor_data.type = SENSOR_DATA_TYPE_SOIL_MOISTURE;
      event.data.sensor_data.payload.soil_moisture = soil_data;
      event.data.sensor_data.timestamp_us =
          (uint64_t)tv_now.tv_sec * 1000000L + (uint64_t)tv_now.tv_usec;
      event_bus_post(&event, pdMS_TO_TICKS(EVENT_BUS_POST_TIMEOUT_MS));
    } else {
      ESP_LOGE(TAG, "Failed to read soil moisture");
    }
    vTaskDelayUntil(&last_wake_time,
                    pdMS_TO_TICKS(SOIL_SENSOR_READ_INTERVAL_MS));
  }
}

esp_err_t app_sensor_tasks_start(void) {
  if (xTaskCreate(temp_humidity_sensor_task, "temp_humidity_sensor_task",
                  TASK_STACK_TEMP_SENSOR, NULL, TASK_PRIO_TEMP_SENSOR,
                  NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create temp_humidity_sensor_task");
    return ESP_FAIL;
  }

  if (xTaskCreate(light_sensor_task, "light_sensor_task",
                  TASK_STACK_LIGHT_SENSOR, NULL, TASK_PRIO_LIGHT_SENSOR,
                  NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create light_sensor_task");
    return ESP_FAIL;
  }

  if (xTaskCreate(soil_moisture_sensor_task, "soil_moisture_sensor_task",
                  TASK_STACK_SOIL_SENSOR, NULL, TASK_PRIO_SOIL_SENSOR,
                  NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create soil_moisture_sensor_task");
    return ESP_FAIL;
  }

  return ESP_OK;
}
