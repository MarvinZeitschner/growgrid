#include "mqtt_manager.h"
#include "app_config.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "mqtt_client.h"
#include "secrets.h"
#include "sensor_data.h"
#include <string.h>

#define ALL_PUBS_ACKED_BIT (1 << 0)

static const char *TAG = "MQTT_MANAGER";

static esp_mqtt_client_handle_t s_client = NULL;
static EventGroupHandle_t s_mqtt_event_group = NULL;
static int s_outstanding_pub_acks = 0;
static bool s_mqtt_connected = false;

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

static void mqtt_publisher_task(void *pvParameter) {
  ESP_LOGI(TAG, "MQTT publisher task started.");
  task_params_t config = *(task_params_t *)pvParameter;

  SensorData_t local_sensor_data_copy;
  sensor_data_init(&local_sensor_data_copy);

  while (1) {
    EventBits_t bits = xEventGroupWaitBits(
        config.event_group, EVENT_SENSOR_ALL_BITS, pdTRUE, pdTRUE,
        pdMS_TO_TICKS(EVENT_SENSOR_WAIT_TIMEOUT_MS));

    if (xSemaphoreTake(config.data_mutex,
                       pdMS_TO_TICKS(SHARED_DATA_SEMAPHORE_TIMEOUT_MS)) ==
        pdTRUE) {
      local_sensor_data_copy = *config.shared_sensor_data;
      xSemaphoreGive(config.data_mutex);
    } else {
      ESP_LOGE(TAG, "Failed to acquire mutex for shared sensor data.");
    }

    if (mqtt_manager_is_connected()) {
      if (bits & EVENT_SENSOR_TEMP_BIT) {
        publish_float_reading("temperature",
                              local_sensor_data_copy.temperature);
      }
      if (bits & EVENT_SENSOR_LUX_BIT) {
        publish_int_reading("light", local_sensor_data_copy.light);
      }
      if (bits & EVENT_SENSOR_SOIL_BIT) {
        publish_int_reading("soil_moisture",
                            local_sensor_data_copy.soil_moisture);
      }
    } else {
      ESP_LOGW(TAG, "MQTT not connected, skipping publish.");
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    s_outstanding_pub_acks = 0;
    xEventGroupSetBits(s_mqtt_event_group, ALL_PUBS_ACKED_BIT);
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
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    s_outstanding_pub_acks--;
    if (s_outstanding_pub_acks == 0) {
      xEventGroupSetBits(s_mqtt_event_group, ALL_PUBS_ACKED_BIT);
    }
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
  s_mqtt_event_group = xEventGroupCreate();
  esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  return esp_mqtt_client_start(s_client);
}

esp_err_t mqtt_manager_publish(const char *topic, const char *data) {
  if (!s_mqtt_connected) {
    ESP_LOGE(TAG, "MQTT client not connected, cannot publish.");
    return ESP_FAIL;
  }
  int msg_id = esp_mqtt_client_publish(s_client, topic, data, 0, 1, 0);
  if (msg_id > 0) {
    s_outstanding_pub_acks++;
    xEventGroupClearBits(s_mqtt_event_group, ALL_PUBS_ACKED_BIT);
    return ESP_OK;
  } else {
    return ESP_FAIL;
  }
}

bool mqtt_manager_is_connected(void) { return s_mqtt_connected; }

esp_err_t mqtt_manager_start_publisher(QueueHandle_t data_queue,
                                       EventGroupHandle_t event_group,
                                       SemaphoreHandle_t data_mutex,
                                       SensorData_t *shared_data) {
  static task_params_t params;
  params.data_queue = data_queue;
  params.event_group = event_group;
  params.data_mutex = data_mutex;
  params.shared_sensor_data = shared_data;

  xTaskCreate(&mqtt_publisher_task, "mqtt_publisher",
              TASK_STACK_POWER_PUBLISHER, &params, TASK_PRIO_POWER_MANGER,
              NULL);
  return ESP_OK;
}
