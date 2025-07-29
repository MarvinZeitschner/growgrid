#include "power_manager.h"
#include "app_config.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/idf_additions.h"
#include "mqtt_manager.h"
#include "sensor_data.h"
#include "soil_sensor_service.h"
#include "wifi_manager.h"

static const char *TAG = "POWER_MANAGER";

typedef struct {
  EventGroupHandle_t event_group;
  QueueHandle_t data_queue;
  SemaphoreHandle_t data_mutex;
  SensorData_t *shared_sensor_data;
} task_params_t;

static void publish_float_reading(const char *sensor_name, float value) {
  char topic[64];
  snprintf(topic, sizeof(topic), "growgrid/telemetry/%s", sensor_name);

  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", value);

  ESP_LOGI(TAG, "Publishing to %s: %s", topic, payload);
  mqtt_manager_publish(topic, payload);
}

static void publish_int_reading(const char *sensor_name, int value) {
  char topic[64];
  snprintf(topic, sizeof(topic), "growgrid/telemetry/%s", sensor_name);

  char payload[16];
  snprintf(payload, sizeof(payload), "%d", value);

  ESP_LOGI(TAG, "Publishing to %s: %s", topic, payload);
  mqtt_manager_publish(topic, payload);
}

static void power_manager_task(void *pvParameter) {
  ESP_LOGI(TAG, "Power manager task started.");
  task_params_t config = *(task_params_t *)pvParameter;

  SensorData_t local_sensor_data_copy;
  sensor_data_init(&local_sensor_data_copy);

  uint8_t soil_sensor_wake_count = soil_sensor_get_wake_count();
  EventBits_t bits_to_wait_for;
  if (soil_sensor_wake_count % 3 == 0) {
    bits_to_wait_for = EVENT_SENSOR_ALL_BITS;
  } else {
    bits_to_wait_for = EVENT_SENSOR_FAST_BITS;
  }

  EventBits_t bits =
      xEventGroupWaitBits(config.event_group, bits_to_wait_for, pdTRUE, pdTRUE,
                          pdMS_TO_TICKS(EVENT_SENSOR_WAIT_TIMEOUT_MS));

  if (xSemaphoreTake(config.data_mutex,
                     pdMS_TO_TICKS(SHARED_DATA_SEMAPHORE_TIMEOUT_MS))) {
    local_sensor_data_copy = *config.shared_sensor_data;
    xSemaphoreGive(config.data_mutex);
  }

  if (mqtt_manager_is_connected()) {
    if (bits & EVENT_SENSOR_TEMP_BIT) {
      publish_float_reading("temperature", local_sensor_data_copy.temperature);
    }
    if (bits & EVENT_SENSOR_LUX_BIT) {
      publish_int_reading("light", local_sensor_data_copy.light);
    }
    if (bits & EVENT_SENSOR_SOIL_BIT) {
      publish_int_reading("soil_moisture",
                          local_sensor_data_copy.soil_moisture);
    }

    ESP_LOGI(TAG, "Waiting for all messages to be published...");
    esp_err_t pub_ack_result = mqtt_manager_wait_for_all_publishes(5000);
    if (pub_ack_result == ESP_OK) {
      ESP_LOGI(TAG, "All messages acknowledged.");
    } else {
      ESP_LOGE(
          TAG,
          "Failed to receive all publish acknowledgements within the timeout.");
    }
  }

  ESP_LOGI(TAG, "Disconnecting MQTT client...");
  mqtt_manager_disconnect();

  while (mqtt_manager_is_connected()) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  ESP_LOGI(TAG, "MQTT client disconnected.");

  ESP_LOGI(TAG, "Disconnecting WiFi...");
  wifi_manager_disconnect();
  esp_err_t wifi_dis_result = wifi_manager_wait_for_disconnect(5000);
  if (wifi_dis_result == ESP_OK) {
    ESP_LOGI(TAG, "WiFi disconnected.");
  } else {
    ESP_LOGE(TAG,
             "Failed to receive wifi disconnect event within the timeout.");
  }
  wifi_manager_stop();

  ESP_LOGI(TAG, "Going into deep sleep");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

esp_err_t power_manager_start(QueueHandle_t data_queue,
                              EventGroupHandle_t event_group,
                              SemaphoreHandle_t data_mutex,
                              SensorData_t *shared_data) {
  static task_params_t params;
  params.data_queue = data_queue;
  params.event_group = event_group;
  params.data_mutex = data_mutex;
  params.shared_sensor_data = shared_data;

  xTaskCreate(&power_manager_task, "power_manager", TASK_STACK_POWER_PUBLISHER,
              &params, TASK_PRIO_POWER_MANGER, NULL);

  return ESP_OK;
}
