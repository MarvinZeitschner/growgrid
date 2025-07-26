#include "mqtt.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "secrets.h"

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;
static bool g_mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    g_mqtt_connected = true;
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    g_mqtt_connected = false;
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA: %.*s = %.*s", event->topic_len,
             event->topic, event->data_len, event->data);
    break;
  default:
    break;
  };
}

void mqtt_app_start(void) {
  esp_mqtt_client_config_t mqtt_conf = {
      .broker.address.uri = MQTT_BROKER,
      .credentials.username = MQTT_USER,
      .credentials.authentication.password = MQTT_PASS,
  };

  client = esp_mqtt_client_init(&mqtt_conf);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(client);
}

esp_mqtt_client_handle_t get_mqtt_client(void) { return client; }

bool mqtt_is_connected(void) { return g_mqtt_connected; }
