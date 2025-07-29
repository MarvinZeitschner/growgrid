#include "mqtt_manager.h"
#include "app_config.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "mqtt_client.h"
#include "secrets.h"
#include "sensor_data.h"
#include "soil_sensor_service.h"
#include <stdint.h>
#include <string.h>

static const char *TAG = "MQTT_MANAGER";

typedef struct {
  EventGroupHandle_t event_group;
  QueueHandle_t data_queue;
  SemaphoreHandle_t data_mutex;
  SensorData_t *shared_sensor_data;

  esp_mqtt_client_handle_t client;
} task_params_t;

static const char *TOPIC = "growgrid/telemetry";

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

static void publish_float_reading(esp_mqtt_client_handle_t client,
                                  const char *sensor_name, float value) {
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/%s", TOPIC, sensor_name);

  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", value);

  ESP_LOGI(TAG, "Publishing to %s: %s", topic, payload);
  esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
}

static void publish_int_reading(esp_mqtt_client_handle_t client,
                                const char *sensor_name, int value) {
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/%s", TOPIC, sensor_name);

  char payload[16];
  snprintf(payload, sizeof(payload), "%d", value);

  ESP_LOGI(TAG, "Publishing to %s: %s", topic, payload);
  esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
}

static void mqtt_publisher_task(void *pvParameter) {
  ESP_LOGI(TAG, "MQTT Publisher task started.");
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

  if (s_mqtt_connected) {
    if (bits & EVENT_SENSOR_TEMP_BIT) {
      publish_float_reading(config.client, "temperature",
                            local_sensor_data_copy.temperature);
    }
    if (bits & EVENT_SENSOR_LUX_BIT) {
      publish_int_reading(config.client, "light", local_sensor_data_copy.light);
    }
    if (bits & EVENT_SENSOR_SOIL_BIT) {
      publish_int_reading(config.client, "soil_moisture",
                          local_sensor_data_copy.soil_moisture);
    }
  }

  vTaskDelay(pdMS_TO_TICKS(2000));
  esp_mqtt_client_disconnect(config.client);

  ESP_LOGI(TAG, "Going into deep sleep");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

esp_err_t mqtt_manager_start(QueueHandle_t data_queue,
                             EventGroupHandle_t event_group,
                             SemaphoreHandle_t data_mutex,
                             SensorData_t *shared_data) {
  static task_params_t params;
  params.data_queue = data_queue;
  params.event_group = event_group;
  params.data_mutex = data_mutex;
  params.shared_sensor_data = shared_data;

  esp_mqtt_client_config_t mqtt_conf = {
      .broker.address.uri = MQTT_BROKER,
      .credentials.username = MQTT_USER,
      .credentials.authentication.password = MQTT_PASS,
  };

  params.client = esp_mqtt_client_init(&mqtt_conf);
  esp_mqtt_client_register_event(params.client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, NULL);
  esp_mqtt_client_start(params.client);

  xTaskCreate(&mqtt_publisher_task, "mqtt_publisher", TASK_STACK_MQTT_PUBLISHER,
              &params, TASK_PRIO_MQTT_PUBLISHER, NULL);

  return ESP_OK;
}

bool mqtt_manager_is_connected(void) { return s_mqtt_connected; }
