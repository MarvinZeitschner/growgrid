#include "app_config.h"
#include "bus_manager.h"
#include "esp_log.h"
#include "fast_sensor_service.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "mqtt_manager.h"
#include "nvs_flash.h"
#include "power_manager.h"
#include "sensor_data.h"
#include "soil_sensor_service.h"
#include "wifi_manager.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "Starting GrowGrid application...");

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "Initializing WiFi...");
  ESP_ERROR_CHECK(wifi_manager_init_sta());
  ESP_LOGI(TAG, "WiFi connected.");

  ESP_LOGI(TAG, "Initializing hardware buses...");
  ESP_ERROR_CHECK(bus_manager_init_i2c());
  i2c_bus_handle_t i2c_handle = bus_manager_get_i2c_handle();

  QueueHandle_t data_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(float));
  if (data_queue == NULL) {
    ESP_LOGE(TAG, "Failed to create data queue.");
    while (1) {
    }
  }

  EventGroupHandle_t sensor_event_group = xEventGroupCreate();
  if (sensor_event_group == NULL) {
    ESP_LOGE(TAG, "Failed to create event group.");
    while (1) {
    }
  }

  SemaphoreHandle_t data_mutex = xSemaphoreCreateMutex();

  static SensorData_t shared_sensor_data;
  sensor_data_init(&shared_sensor_data);

  ESP_LOGI(TAG, "Starting sensor services...");
  ESP_ERROR_CHECK(fast_sensor_service_start(i2c_handle, data_queue,
                                            sensor_event_group, data_mutex,
                                            &shared_sensor_data));
  ESP_ERROR_CHECK(soil_sensor_service_start(data_queue, sensor_event_group,
                                            data_mutex, &shared_sensor_data));

  ESP_LOGI(TAG, "Initializing MQTT manager...");
  ESP_ERROR_CHECK(mqtt_manager_start());

  ESP_LOGI(TAG, "Starting power manager...");
  ESP_ERROR_CHECK(power_manager_start(data_queue, sensor_event_group,
                                      data_mutex, &shared_sensor_data));

  ESP_LOGI(TAG, "Application startup complete. System is running.");
}
