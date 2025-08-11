#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/queue.h"
#include "sensor_data.h"
#include <stdbool.h>

/**
 * @brief Starts the MQTT client.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t mqtt_manager_start(void);

/**
 * @brief Starts the MQTT publisher task.
 *
 * @param data_queue Handle to the data queue.
 * @param event_group Handle to the event group.
 * @param data_mutex Handle to the data mutex.
 * @param shared_data Pointer to the shared sensor data structure.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t mqtt_manager_start_publisher(QueueHandle_t data_queue,
                                       EventGroupHandle_t event_group,
                                       SemaphoreHandle_t data_mutex,
                                       SensorData_t *shared_data);

/**
 * @brief Publishes a message to a given topic.
 *
 * @param topic The topic to publish to.
 * @param data The data to publish.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t mqtt_manager_publish(const char *topic, const char *data);

/**
 * @brief Checks if the MQTT client is currently connected.
 *
 * @return true if connected, false otherwise.
 */
bool mqtt_manager_is_connected(void);