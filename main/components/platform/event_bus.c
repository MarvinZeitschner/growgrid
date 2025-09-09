#include "event_bus.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <string.h>

/**
 * All of this will soon change to the official esp event loop library
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_event.html
 */

#define MAX_SUBSCRIBERS 10
#define EVENT_BUS_QUEUE_SIZE 32

static const char *TAG = "EVENT_BUS";

static QueueHandle_t s_subscriber_queues[MAX_SUBSCRIBERS];
static SemaphoreHandle_t s_subscriber_list_mutex;
static QueueHandle_t s_event_bus_queue;

static void event_distributor_task(void *arg) {
  event_t event;
  while (1) {
    if (xQueueReceive(s_event_bus_queue, &event, portMAX_DELAY) == pdTRUE) {
      if (xSemaphoreTake(s_subscriber_list_mutex, portMAX_DELAY) == pdTRUE) {
        for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
          if (s_subscriber_queues[i] != NULL) {
            if (xQueueSend(s_subscriber_queues[i], &event, 0) != pdTRUE) {
              ESP_LOGW(TAG,
                       "Subscriber queue full, event dropped for a subscriber");
            }
          }
        }
        xSemaphoreGive(s_subscriber_list_mutex);
      }
    }
  }
}

esp_err_t event_bus_init(void) {
  memset(s_subscriber_queues, 0, sizeof(s_subscriber_queues));

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
  QueueHandle_t new_queue = NULL;

  if (xSemaphoreTake(s_subscriber_list_mutex, portMAX_DELAY) == pdTRUE) {
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
      if (s_subscriber_queues[i] == NULL) {
        new_queue = xQueueCreate(EVENT_BUS_QUEUE_SIZE, sizeof(event_t));
        if (new_queue == NULL) {
          ESP_LOGE(TAG, "Failed to create subscriber queue");
          break;
        }
        s_subscriber_queues[i] = new_queue;
        ESP_LOGI(TAG, "New subscriber added to slot %d", i);
        break;
      }
    }
    xSemaphoreGive(s_subscriber_list_mutex);

    if (new_queue == NULL) {
      ESP_LOGE(TAG, "Failed to add new subscriber, all slots are full.");
    }
  }
  return new_queue;
}

void event_bus_unsubscribe(QueueHandle_t queue) {
  if (xSemaphoreTake(s_subscriber_list_mutex, portMAX_DELAY) == pdTRUE) {
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
      if (s_subscriber_queues[i] == queue) {
        vQueueDelete(s_subscriber_queues[i]);
        s_subscriber_queues[i] = NULL;
        ESP_LOGI(TAG, "Subscriber removed from slot %d", i);
        break;
      }
    }
    xSemaphoreGive(s_subscriber_list_mutex);
  }
}
