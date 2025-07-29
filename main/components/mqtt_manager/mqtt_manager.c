#include "mqtt_manager.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "secrets.h"
#include <string.h>

static const char *TAG = "MQTT_MANAGER";

static esp_mqtt_client_handle_t s_client = NULL;
static bool s_mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    s_mqtt_connected = true;
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    s_mqtt_connected = false;
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA: %.*s = %.*s", event->topic_len,
             event->topic, event->data_len, event->data);
    break;
  default:
    break;
  }
}

esp_err_t mqtt_manager_start(void) {
  esp_mqtt_client_config_t mqtt_conf = {
      .broker.address.uri = MQTT_BROKER,
      .credentials.username = MQTT_USER,
      .credentials.authentication.password = MQTT_PASS,
  };

  s_client = esp_mqtt_client_init(&mqtt_conf);
  esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  return esp_mqtt_client_start(s_client);
}

esp_err_t mqtt_manager_disconnect(void) {
  return esp_mqtt_client_disconnect(s_client);
}

esp_err_t mqtt_manager_publish(const char *topic, const char *data) {
  if (!s_mqtt_connected) {
    ESP_LOGE(TAG, "MQTT client not connected, cannot publish.");
    return ESP_FAIL;
  }
  int msg_id = esp_mqtt_client_publish(s_client, topic, data, 0, 1, 0);
  return (msg_id > 0) ? ESP_OK : ESP_FAIL;
}

bool mqtt_manager_is_connected(void) { return s_mqtt_connected; }
