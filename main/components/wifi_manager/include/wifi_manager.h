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

/**
 * @brief Waits for the WiFi to be fully disconnected.
 *
 * @param timeout_ms The maximum time to wait in milliseconds.
 * @return ESP_OK on success, ESP_ERR_TIMEOUT on timeout.
 */
esp_err_t wifi_manager_wait_for_disconnect(uint32_t timeout_ms);

/**
 * @brief Stops the WiFi service.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t wifi_manager_stop(void);