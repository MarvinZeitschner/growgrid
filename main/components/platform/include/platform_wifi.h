#pragma once
#include "esp_err.h"

/**
 * @brief Initializes and starts the WiFi station.
 *
 * Connects to the AP configured in secrets.h. This is a blocking call.
 * It will post events to the event bus on connection/disconnection.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t platform_wifi_init_sta(const char *ssid, const char *pass);
