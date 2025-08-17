#include "event_bus.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <string.h>
#include <sys/queue.h>

#define MAX_SUBSCRIBERS 10
#define EVENT_BUS_QUEUE_SIZE 16

static const char *TAG = "EVENT_BUS";

typedef struct subscriber_node {
  QueueHandle_t queue;
  SLIST_ENTRY(subscriber_node) entries;
} subscriber_node_t;

static SLIST_HEAD(subscriber_list_head, subscriber_node) s_subscriber_list;
static SemaphoreHandle_t s_subscriber_list_mutex;
static QueueHandle_t s_event_bus_queue;

static void event_distributor_task(void *arg) {
  event_t event;
  while (1) {
    if (xQueueReceive(s_event_bus_queue, &event, portMAX_DELAY) == pdTRUE) {
      if (xSemaphoreTake(s_subscriber_list_mutex, portMAX_DELAY) == pdTRUE) {
        subscriber_node_t *node;
        SLIST_FOREACH(node, &s_subscriber_list, entries) {
          if (xQueueSend(node->queue, &event, 0) != pdTRUE) {
            ESP_LOGW(TAG,
                     "Subscriber queue full, event dropped for a subscriber");
          }
        }
        xSemaphoreGive(s_subscriber_list_mutex);
      }
    }
  }
}

esp_err_t event_bus_init(void) {
  SLIST_INIT(&s_subscriber_list);
  s_subscriber_list_mutex = xSemaphoreCreateMutex();
  if (s_subscriber_list_mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create subscriber list mutex");
    return ESP_FAIL;
  }

  s_event_bus_queue = xQueueCreate(EVENT_BUS_QUEUE_SIZE, sizeof(event_t));
  if (s_event_bus_queue == NULL) {
    ESP_LOGE(TAG, "Failed to create event bus queue");
    vSemaphoreDelete(s_subscriber_list_mutex);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Event bus initialized");
  return ESP_OK;
}

esp_err_t event_bus_start_distributor(void) {
  if (xTaskCreate(event_distributor_task, "event_distributor", 4096, NULL, 10,
                  NULL) != pdPASS) {
    ESP_LOGE(TAG, "Failed to create event distributor task");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Event distributor task started");
  return ESP_OK;
}

esp_err_t event_bus_post(const event_t *event, uint32_t timeout_ms) {
  if (xQueueSend(s_event_bus_queue, event, pdMS_TO_TICKS(timeout_ms)) !=
      pdTRUE) {
    ESP_LOGW(TAG, "Event bus queue full, event dropped");
    return ESP_ERR_TIMEOUT;
  }
  return ESP_OK;
}

QueueHandle_t event_bus_subscribe(void) {
  QueueHandle_t new_queue = xQueueCreate(EVENT_BUS_QUEUE_SIZE, sizeof(event_t));
  if (new_queue == NULL) {
    ESP_LOGE(TAG, "Failed to create subscriber queue");
    return NULL;
  }

  subscriber_node_t *new_node = malloc(sizeof(subscriber_node_t));
  if (new_node == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for subscriber node");
    vQueueDelete(new_queue);
    return NULL;
  }
  new_node->queue = new_queue;

  if (xSemaphoreTake(s_subscriber_list_mutex, portMAX_DELAY) == pdTRUE) {
    SLIST_INSERT_HEAD(&s_subscriber_list, new_node, entries);
    xSemaphoreGive(s_subscriber_list_mutex);
  }

  ESP_LOGI(TAG, "New subscriber added");
  return new_queue;
}

void event_bus_unsubscribe(QueueHandle_t queue) {
  if (xSemaphoreTake(s_subscriber_list_mutex, portMAX_DELAY) == pdTRUE) {
    subscriber_node_t *node, *temp;
    SLIST_FOREACH_SAFE(node, &s_subscriber_list, entries, temp) {
      if (node->queue == queue) {
        SLIST_REMOVE(&s_subscriber_list, node, subscriber_node, entries);
        vQueueDelete(node->queue);
        free(node);
        break;
      }
    }
    xSemaphoreGive(s_subscriber_list_mutex);
  }
  ESP_LOGI(TAG, "Subscriber removed");
}
