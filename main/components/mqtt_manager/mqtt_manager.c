#include "mqtt_manager.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/timers.h"
#include "mqtt_client.h"
#include "secrets.h"
#include "sensor_aggregator.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MQTT_MANAGER";

static esp_mqtt_client_handle_t s_client = NULL;
static QueueHandle_t s_sensor_data_queue;
static bool s_mqtt_connected = false;
static TimerHandle_t s_reconnect_timer = NULL;

static void reconnect_timer_callback(TimerHandle_t xTimer) {
  ESP_LOGE(
      TAG,
      "MQTT client has been disconnected for too long. Restarting device.");
  esp_restart();
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    s_mqtt_connected = true;
    if (s_reconnect_timer != NULL) {
      xTimerStop(s_reconnect_timer, 0);
      xTimerDelete(s_reconnect_timer, 0);
      s_reconnect_timer = NULL;
      ESP_LOGI(TAG, "Reconnected successfully. Reconnect timer cancelled.");
    }
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    s_mqtt_connected = false;
    if (s_reconnect_timer == NULL) {
      ESP_LOGW(TAG, "MQTT disconnected. Starting 10-minute reconnect timer.");
      s_reconnect_timer =
          xTimerCreate("ReconnectTimer", pdMS_TO_TICKS(10 * 60 * 1000), pdFALSE,
                       (void *)0, reconnect_timer_callback);
      if (s_reconnect_timer != NULL)
        xTimerStart(s_reconnect_timer, 0);
    }
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA: %.*s = %.*s", event->topic_len,
             event->topic, event->data_len, event->data);
    break;
  default:
    break;
  }
}

static void mqtt_publisher_task(void *pvParameter) {
  ESP_LOGI(TAG, "MQTT Publisher task started.");
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

  sensor_data_t data;
  char topic[64];
  char payload[16];

  while (1) {
    esp_task_wdt_reset();
    if (xQueueReceive(s_sensor_data_queue, &data, portMAX_DELAY) == pdPASS) {
      if (s_mqtt_connected) {
        switch (data.type) {
        case DATA_TYPE_LUX:
          strcpy(topic, "growgrid/light");
          snprintf(payload, sizeof(payload), "%" PRIu32, (uint32_t)data.value);
          break;
        case DATA_TYPE_TEMPERATURE:
          strcpy(topic, "growgrid/temperature");
          snprintf(payload, sizeof(payload), "%.2f", data.value);
          break;
        case DATA_TYPE_SOIL_MOISTURE:
          strcpy(topic, "growgrid/soil_moisture");
          snprintf(payload, sizeof(payload), "%d", (int)data.value);
          break;
        }
        ESP_LOGI(TAG, "Publishing to %s: %s", topic, payload);
        esp_mqtt_client_publish(s_client, topic, payload, 0, 1, 0);
      }
    }
  }
}

esp_err_t mqtt_manager_start(QueueHandle_t queue) {
  s_sensor_data_queue = queue;

  esp_mqtt_client_config_t mqtt_conf = {
      .broker.address.uri = MQTT_BROKER,
      .credentials.username = MQTT_USER,
      .credentials.authentication.password = MQTT_PASS,
  };

  s_client = esp_mqtt_client_init(&mqtt_conf);
  esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(s_client);

  xTaskCreate(&mqtt_publisher_task, "mqtt_publisher", 4096, NULL, 5, NULL);

  return ESP_OK;
}

bool mqtt_manager_is_connected(void) { return s_mqtt_connected; }
