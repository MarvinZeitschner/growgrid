#include "app_config.h"
#include "bmp280_service.h"
#include "bus_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "mqtt_manager.h"
#include "nvs_flash.h"
#include "rgb_led.h"
#include "soil_sensor_service.h"
#include "tsl2561_service.h"
#include "wifi_manager.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "Starting GrowGrid application...");

  // Initialize NVS Flash
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Indicate startup
  ESP_ERROR_CHECK(rgb_led_default_init());
  ESP_ERROR_CHECK(rgb_led_set_color(0, 0, 255));

  // Start WiFi and wait for connection
  ESP_LOGI(TAG, "Initializing WiFi...");
  ESP_ERROR_CHECK(wifi_manager_init_sta());
  ESP_LOGI(TAG, "WiFi connected.");

  // Initialize shared hardware buses
  ESP_LOGI(TAG, "Initializing hardware buses...");
  ESP_ERROR_CHECK(bus_manager_init_i2c());
  i2c_bus_handle_t i2c_handle = bus_manager_get_i2c_handle();

  QueueHandle_t temp_data_queue =
      xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(float));
  if (temp_data_queue == NULL) {
    ESP_LOGE(TAG, "Failed to create temp data queue.");
    esp_restart();
  }
  QueueHandle_t lux_data_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(int));
  if (lux_data_queue == NULL) {
    ESP_LOGE(TAG, "Failed to create lux data queue.");
    esp_restart();
  }
  QueueHandle_t soil_data_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(int));
  if (soil_data_queue == NULL) {
    ESP_LOGE(TAG, "Failed to create soil data queue.");
    esp_restart();
  }

  // Start the MQTT manager
  ESP_LOGI(TAG, "Initializing MQTT manager...");
  ESP_ERROR_CHECK(
      mqtt_manager_start(temp_data_queue, lux_data_queue, soil_data_queue));

  // Start all the individual sensor services, injecting their dependencies
  ESP_LOGI(TAG, "Starting sensor services...");
  ESP_ERROR_CHECK(bmp280_service_start(i2c_handle, temp_data_queue));
  ESP_ERROR_CHECK(tsl2561_service_start(i2c_handle, lux_data_queue));
  ESP_ERROR_CHECK(soil_sensor_service_start(soil_data_queue));

  // Indicate successful startup
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(rgb_led_set_color(0, 255, 0)); // Green for running
  vTaskDelay(pdMS_TO_TICKS(2000));
  ESP_ERROR_CHECK(rgb_led_clear());

  ESP_LOGI(TAG, "Application startup complete. System is running.");
}
