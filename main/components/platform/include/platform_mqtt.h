#pragma once

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Initializes and starts the MQTT client and the publisher task.
 *
 * The publisher task subscribes to the event bus and publishes sensor data.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t platform_mqtt_init(const char *broker_uri, const char *username, const char *password);

/**
 * @brief Checks if the MQTT client is currently connected.
 *
 * @return true if connected, false otherwise.
 */
bool platform_mqtt_is_connected(void);
