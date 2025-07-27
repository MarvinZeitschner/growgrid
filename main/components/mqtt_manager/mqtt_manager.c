#include "mqtt_manager.h"
#include "app_config.h"
#include "cJSON.h"
#include "esp_log.h"
// #include "esp_task_wdt.h"
#include "board.h"
#include "esp_sleep.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "mqtt_client.h"
#include "secrets.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "MQTT_MANAGER";

static const char *TOPIC = "growgrid/telemetry";

static esp_mqtt_client_handle_t s_client = NULL;
static bool s_mqtt_connected = false;
static TimerHandle_t s_reconnect_timer = NULL;

static QueueSetHandle_t s_temp_queue = NULL;
static QueueSetHandle_t s_lux_queue = NULL;
static QueueSetHandle_t s_soil_queue = NULL;
static EventGroupHandle_t s_event_group = NULL;

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

  while (!mqtt_manager_is_connected()) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  float temperature = -999.f;
  int lux = -1;
  int soil = -1;

  // TickType_t x_last_wake_time = xTaskGetTickCount();

  // while (1) {
  EventBits_t bits =
      xEventGroupWaitBits(s_event_group, EVENT_SENSOR_ALL_BITS, pdTRUE, pdTRUE,
                          pdMS_TO_TICKS(EVENT_SENSOR_WAIT_TIMEOUT_MS));

  if (bits & EVENT_SENSOR_TEMP_BIT)
    xQueuePeek(s_temp_queue, &temperature, 0);
  else
    ESP_LOGW(TAG, "Temperature sensor did not respond!");

  if (bits & EVENT_SENSOR_LUX_BIT)
    xQueuePeek(s_lux_queue, &lux, 0);
  else
    ESP_LOGW(TAG, "Lux sensor did not respond!");

  if (bits & EVENT_SENSOR_SOIL_BIT)
    xQueuePeek(s_soil_queue, &soil, 0);
  else
    ESP_LOGW(TAG, "Soil sensor did not respond!");

  if (s_mqtt_connected) {
    cJSON *root = cJSON_CreateObject();
    if (temperature > -999.f)
      cJSON_AddNumberToObject(root, "temperature", temperature);
    if (lux > -1)
      cJSON_AddNumberToObject(root, "light", lux);
    if (soil > -1)
      cJSON_AddNumberToObject(root, "soil_moisture", soil);

    char *json_string = cJSON_Print(root);
    if (json_string) {
      ESP_LOGI(TAG, "Publishing: %s", json_string);
      esp_mqtt_client_publish(s_client, TOPIC, json_string, 0, 1, 0);
      free(json_string);
    }
    cJSON_Delete(root);
  }

  vTaskDelay(pdMS_TO_TICKS(2000));

  ESP_LOGI(TAG, "Going into deep sleep");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * 1000000ULL);
  esp_deep_sleep_start();

  // vTaskDelayUntil(&x_last_wake_time,
  // pdMS_TO_TICKS(MQTT_PUBLISH_INTERVAL_MS));
  // }
}

esp_err_t mqtt_manager_start(QueueSetHandle_t temp_queue,
                             QueueSetHandle_t lux_queue,
                             QueueSetHandle_t soil_queue,
                             EventGroupHandle_t event_group) {
  s_temp_queue = temp_queue;
  s_lux_queue = lux_queue;
  s_soil_queue = soil_queue;
  s_event_group = event_group;

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
