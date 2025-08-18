#include "app_controller.h"
#include "esp_log.h"
#include "event_bus.h"
#include "hal_i2c.h"
#include "hal_pump.h"
#include "hal_sensors.h"
#include "nvs_flash.h"
#include "platform_mqtt.h"
#include "platform_sntp.h"
#include "platform_wifi.h"
#include "provisioning.h"
#include "pump_control_task.h"
#include "sensor_tasks.h"
#include "storage.h"

static const char *TAG = "APP_CONTROLLER";

static void start_application(const credentials_t *creds) {
  ESP_ERROR_CHECK(event_bus_init());
  ESP_ERROR_CHECK(event_bus_start_distributor());

  ESP_LOGI(TAG, "Initializing WiFi...");
  ESP_ERROR_CHECK(platform_wifi_init_sta(creds->wifi_ssid, creds->wifi_pass));
  ESP_LOGI(TAG, "WiFi connected.");

  ESP_LOGI(TAG, "Initializing SNTP...");
  platform_sntp_init(creds->ntp_server);

  ESP_LOGI(TAG, "Initializing MQTT...");
  ESP_ERROR_CHECK(platform_mqtt_init(creds->mqtt_broker, creds->mqtt_user,
                                     creds->mqtt_pass));

  ESP_LOGI(TAG, "Initializing HAL...");
  ESP_ERROR_CHECK(hal_i2c_init());
  ESP_ERROR_CHECK(hal_pump_init());
  ESP_ERROR_CHECK(hal_sensors_init());
  ESP_LOGI(TAG, "HAL initialized.");

  ESP_LOGI(TAG, "Starting application tasks...");
  ESP_ERROR_CHECK(app_sensor_tasks_start());
  ESP_ERROR_CHECK(app_pump_control_task_start());
  ESP_LOGI(TAG, "Application tasks started.");

  ESP_LOGI(TAG, "Application startup complete. System is running.");
}

esp_err_t app_controller_init(void) {
  ESP_LOGI(TAG, "Initializing application controller...");

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  credentials_t creds;
  if (storage_read_credentials(&creds) == ESP_OK) {
    ESP_LOGI(TAG, "Credentials found in NVS. Starting application.");
    start_application(&creds);
  } else {
    ESP_LOGI(TAG, "Credentials not found. Starting provisioning mode.");
    provisioning_start();
  }

  return ESP_OK;
}
