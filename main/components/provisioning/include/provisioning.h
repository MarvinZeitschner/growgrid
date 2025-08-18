#pragma once

#include "esp_err.h"

/**
 * @brief Starts the provisioning mode.
 *
 * Initializes SoftAP and starts an HTTP server to get credentials.
 * This function does not return until provisioning is complete.
 * After successful provisioning, the device will restart.
 *
 * @return ESP_OK on success (though it will restart before returning).
 */
esp_err_t provisioning_start(void);
