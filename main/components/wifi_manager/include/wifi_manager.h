#pragma once

#include "esp_err.h"

/**
 * @brief Initializes the WiFi station, connects to the configured AP,
 *        and waits until an IP address is obtained.
 *
 * This function is synchronous (blocking). It will not return until a
 * connection is established or the connection fails after multiple retries.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t wifi_manager_init_sta(void);

/**
 * @brief Disconnects the WiFi station.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t wifi_manager_disconnect(void);