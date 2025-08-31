#include "platform_mqtt.h"
#include "app_config.h"
#include "cJSON.h"
#include "esp_log.h"
#include "event_bus.h"
#include "mqtt_client.h"
#include "cert_store.h" // Include the new component header

#include <stdio.h>

static const char *TAG = "PLATFORM_MQTT";
static esp_mqtt_client_handle_t s_client = NULL;
static bool s_mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    s_mqtt_connected = true;
    event_t mqtt_event = {.type = EVENT_TYPE_MQTT_CONNECTED};
    event_bus_post(&mqtt_event, 0);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    s_mqtt_connected = false;
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    break;
  }
}

static void publish_sensor_data(const sensor_data_t *data) {
  if (!s_mqtt_connected) {
    return;
  }

  char topic[128];
  cJSON *root = NULL;
  char *payload_str = NULL;

  switch (data->type) {
  case SENSOR_DATA_TYPE_TEMP_HUMIDITY:
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "value",
                            data->payload.temp_humidity.temperature);
    cJSON_AddNumberToObject(root, "timestamp_us", (double)data->timestamp_us);
    payload_str = cJSON_PrintUnformatted(root);
    snprintf(topic, sizeof(topic), "growgrid/telemetry/temperature");
    esp_mqtt_client_publish(s_client, topic, payload_str, 0, 1, 0);
    free(payload_str);
    cJSON_Delete(root);

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "value",
                            data->payload.temp_humidity.humidity);
    cJSON_AddNumberToObject(root, "timestamp_us", (double)data->timestamp_us);
    payload_str = cJSON_PrintUnformatted(root);
    snprintf(topic, sizeof(topic), "growgrid/telemetry/humidity");
    esp_mqtt_client_publish(s_client, topic, payload_str, 0, 1, 0);
    free(payload_str);
    cJSON_Delete(root);
    break;

  case SENSOR_DATA_TYPE_LIGHT:
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "value", data->payload.light.lux);
    cJSON_AddNumberToObject(root, "timestamp_us", (double)data->timestamp_us);
    payload_str = cJSON_PrintUnformatted(root);
    snprintf(topic, sizeof(topic), "growgrid/telemetry/light");
    esp_mqtt_client_publish(s_client, topic, payload_str, 0, 1, 0);
    free(payload_str);
    cJSON_Delete(root);
    break;

  case SENSOR_DATA_TYPE_SOIL_MOISTURE:
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "value", data->payload.soil_moisture.percent);
    cJSON_AddNumberToObject(root, "timestamp_us", (double)data->timestamp_us);
    payload_str = cJSON_PrintUnformatted(root);
    snprintf(topic, sizeof(topic), "growgrid/telemetry/soil_moisture");
    esp_mqtt_client_publish(s_client, topic, payload_str, 0, 1, 0);
    free(payload_str);
    cJSON_Delete(root);
    break;
  }
}

static void mqtt_publisher_task(void *pvParameters) {
  QueueHandle_t event_queue = event_bus_subscribe();
  if (event_queue == NULL) {
    ESP_LOGE(TAG, "Failed to subscribe to event bus");
    vTaskDelete(NULL);
  }

  ESP_LOGI(TAG, "MQTT publisher task started");

  while (1) {
    event_t event;
    if (xQueueReceive(event_queue, &event, portMAX_DELAY) == pdTRUE) {
      if (event.type == EVENT_TYPE_SENSOR_DATA) {
        publish_sensor_data(&event.data.sensor_data);
      }
    }
  }
}

esp_err_t platform_mqtt_init(const char *broker_uri, const char *username, const char *password) {
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = broker_uri,
      .credentials.username = username,
      .credentials.authentication.password = password,
      .broker.verification.certificate = (const char *)ca_cert_pem_start,
      .credentials.authentication.certificate = (const char *)client_cert_pem_start,
      .credentials.authentication.key = (const char *)client_key_pem_start,
      .session.keepalive = 30,
  };

  s_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(s_client);

  if (xTaskCreate(mqtt_publisher_task, "mqtt_publisher_task",
                  TASK_STACK_MQTT_PUBLISHER, NULL, TASK_PRIO_MQTT_MANGER,
                  NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create MQTT publisher task");
    return ESP_FAIL;
  }

  return ESP_OK;
}

bool platform_mqtt_is_connected(void) { return s_mqtt_connected; }