#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "growgrid_types.h"

typedef enum {
  EVENT_TYPE_SENSOR_DATA,
  EVENT_TYPE_WIFI_CONNECTED,
  EVENT_TYPE_WIFI_DISCONNECTED,
  EVENT_TYPE_MQTT_CONNECTED,
  EVENT_TYPE_PUMP_STATE_CHANGE,
} event_type_t;

typedef struct {
  bool is_on;
} pump_state_event_data_t;

typedef struct {
  event_type_t type;
  union {
    sensor_data_t sensor_data;
    pump_state_event_data_t pump_state;
  } data;
} event_t;

/**
 * @brief Initializes the event bus.
 *
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t event_bus_init(void);

/**
 * @brief Starts the event distributor task. Must be called after
 * event_bus_init.
 *
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t event_bus_start_distributor(void);

/**
 * @brief Posts an event to the event bus.
 *
 * @param event Pointer to the event to post.
 * @param timeout_ms Timeout in milliseconds to wait for space in the queue.
 * @return esp_err_t ESP_OK on success, ESP_ERR_TIMEOUT if the queue is full.
 */
esp_err_t event_bus_post(const event_t *event, uint32_t timeout_ms);

/**
 * @brief Subscribes to the event bus.
 *
 * @return QueueHandle_t A handle to a new queue that will receive all events.
 *         The caller is responsible for deleting this queue. Returns NULL on
 * failure.
 */
QueueHandle_t event_bus_subscribe(void);

/**
 * @brief Unsubscribes from the event bus.
 *
 * @param queue The queue handle returned by event_bus_subscribe.
 */
void event_bus_unsubscribe(QueueHandle_t queue);
