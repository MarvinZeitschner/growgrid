#pragma once

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Starts the MQTT client.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t mqtt_manager_start(void);

/**
 * @brief Disconnects the MQTT client.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t mqtt_manager_disconnect(void);

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