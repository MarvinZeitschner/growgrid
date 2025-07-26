#include "bmp280_service.h"
#include "bmp280.h"
#include "bus_manager.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "sensor_aggregator.h"

static const char *TAG = "BMP280_SERVICE";

static void read_temperature_task(void *pvParameter) {
  QueueHandle_t data_queue = (QueueHandle_t)pvParameter;
  bmp280_handle_t bmp280_handle = NULL;

  i2c_bus_handle_t i2c_handle = bus_manager_get_i2c_handle();
  if (i2c_handle == NULL) {
    ESP_LOGE(TAG, "I2C bus not initialized. Cannot start BMP280 task.");
    vTaskDelete(NULL);
  }

  bmp280_handle = bmp280_create(i2c_handle, BMP280_I2C_ADDRESS_DEFAULT);
  if (bmp280_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create BMP280 handle in task.");
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(bmp280_default_init(bmp280_handle));

  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  sensor_data_t data = {.type = DATA_TYPE_TEMPERATURE};

  // TODO: find a better way to stabilize sensor
  vTaskDelay(pdMS_TO_TICKS(1000));

  while (1) {
    if (bmp280_read_temperature(bmp280_handle, &data.value) == ESP_OK) {
      xQueueSend(data_queue, &data, portMAX_DELAY);
    }
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(1000 * 10));
  }
}

esp_err_t bmp280_service_start(i2c_bus_handle_t i2c_handle,
                               QueueHandle_t data_queue) {
  xTaskCreate(&read_temperature_task, "read_temperature", 2048,
              (void *)data_queue, 4, NULL);
  ESP_LOGI(TAG, "BMP280 service started.");
  return ESP_OK;
}
