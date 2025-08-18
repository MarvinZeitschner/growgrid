#include "storage.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "STORAGE";

esp_err_t storage_save_credentials(const credentials_t *credentials) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  err = nvs_set_blob(nvs_handle, "creds", credentials, sizeof(credentials_t));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) writing credentials to NVS!",
             esp_err_to_name(err));
  } else {
    ESP_LOGI(TAG, "Credentials saved to NVS");
  }

  nvs_commit(nvs_handle);
  nvs_close(nvs_handle);
  return err;
}

esp_err_t storage_read_credentials(credentials_t *credentials) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  size_t required_size = sizeof(credentials_t);
  err = nvs_get_blob(nvs_handle, "creds", credentials, &required_size);
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Error (%s) reading credentials from NVS!",
             esp_err_to_name(err));
  }

  nvs_close(nvs_handle);
  return err;
}
