#pragma once

#include "esp_err.h"

#define STORAGE_NAMESPACE "credentials"

typedef struct {
  char wifi_ssid[32];
  char wifi_pass[64];
  char mqtt_broker[128];
  char mqtt_user[32];
  char mqtt_pass[64];
  char ntp_server[64];
} credentials_t;

/**
 * @brief Saves credentials to NVS.
 *
 * @param credentials Pointer to the credentials struct to save.
 * @return ESP_OK on success.
 */
esp_err_t storage_save_credentials(const credentials_t *credentials);

/**
 * @brief Reads credentials from NVS.
 *
 * @param credentials Pointer to a credentials struct to populate.
 * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if not found.
 */
esp_err_t storage_read_credentials(credentials_t *credentials);