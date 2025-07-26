#include "tsl2561_service.h"
#include "bus_manager.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "sensor_aggregator.h"
#include "tsl2561.h"

static const char *TAG = "TSL2561_SERVICE";

static void read_light_task(void *pvParameter) {
  QueueHandle_t data_queue = (QueueHandle_t)pvParameter;
  tsl2561_handle_t tsl_handle = NULL;

  i2c_bus_handle_t i2c_handle = bus_manager_get_i2c_handle();
  if (i2c_handle == NULL) {
    ESP_LOGE(TAG, "I2C bus not initialized. Cannot start TSL2561 task.");
    vTaskDelete(NULL);
  }

  tsl_handle = tsl2561_create(i2c_handle, TSL2561_I2C_ADDR);
  if (tsl_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create TSL2561 handle in task.");
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(tsl2561_default_init(tsl_handle));

  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  sensor_data_t data = {.type = DATA_TYPE_LUX};
  uint32_t lux = 0;

  while (1) {
    if (tsl2561_read_lux(tsl_handle, &lux) == ESP_OK) {
      data.value = (float)lux;
      xQueueSend(data_queue, &data, portMAX_DELAY);
    }
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(1000 * 10));
  }
}

esp_err_t tsl2561_service_start(i2c_bus_handle_t i2c_handle,
                                QueueHandle_t data_queue) {
  xTaskCreate(&read_light_task, "read_light", 2048, (void *)data_queue, 4,
              NULL);
  ESP_LOGI(TAG, "TSL2561 service started.");
  return ESP_OK;
}
