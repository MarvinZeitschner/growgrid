#include "fast_sensor_service.h"
#include "app_config.h"
#include "bmp280.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "sensor_data.h"
#include "tsl2561.h"

static const char *TAG = "FAST_SENSOR_SERVICE";

typedef struct {
  i2c_bus_handle_t i2c_handle;
  EventGroupHandle_t event_group;
  QueueHandle_t data_queue;
  SemaphoreHandle_t data_mutex;
  SensorData_t *shared_sensor_data;
} task_params_t;

static void fast_sensor_task(void *pvParameter) {
  task_params_t config = *(task_params_t *)pvParameter;

  bmp280_handle_t bmp280_handle =
      bmp280_create(config.i2c_handle, BMP280_I2C_ADDRESS_DEFAULT);
  if (bmp280_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create BMP280 handle in task.");
    bmp280_delete(&bmp280_handle);
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(bmp280_default_init(bmp280_handle));

  tsl2561_handle_t tsl2561_handle =
      tsl2561_create(config.i2c_handle, TSL2561_I2C_ADDR);
  if (tsl2561_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create TSL2561 handle in task.");
    bmp280_delete(&bmp280_handle);
    tsl2561_delete(&tsl2561_handle);
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(tsl2561_default_init(tsl2561_handle));

  ESP_LOGI(TAG, "Initialized BMP280 + TSL2561");

  // Wait for sensors to stabilize
  vTaskDelay(pdMS_TO_TICKS(100));

  while (1) {
    float temp = -999.f;
    int lux = -1;

    esp_err_t temp_res = bmp280_read_temperature(bmp280_handle, &temp);
    esp_err_t lux_res = tsl2561_read_lux(tsl2561_handle, &lux);

    if (temp_res == ESP_OK || lux_res == ESP_OK) {
      if (xSemaphoreTake(config.data_mutex,
                         pdMS_TO_TICKS(SHARED_DATA_SEMAPHORE_TIMEOUT_MS)) ==
          pdTRUE) {
        if (temp_res == ESP_OK) {
          config.shared_sensor_data->temperature = temp;
          xEventGroupSetBits(config.event_group, EVENT_SENSOR_TEMP_BIT);
        }

        if (lux_res == ESP_OK) {
          config.shared_sensor_data->light = lux;
          xEventGroupSetBits(config.event_group, EVENT_SENSOR_LUX_BIT);
        }

        xSemaphoreGive(config.data_mutex);
      } else {
        ESP_LOGE(TAG, "Failed to acquire mutex");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

esp_err_t fast_sensor_service_start(i2c_bus_handle_t i2c_handle,
                                    QueueHandle_t data_queue,
                                    EventGroupHandle_t event_group,
                                    SemaphoreHandle_t data_mutex,
                                    SensorData_t *shared_sensor_data) {
  static task_params_t params;

  params.i2c_handle = i2c_handle;
  params.data_queue = data_queue;
  params.event_group = event_group;
  params.data_mutex = data_mutex;
  params.shared_sensor_data = shared_sensor_data;

  xTaskCreate(&fast_sensor_task, "fast_sensors", TASK_STACK_SENSOR_SERVICE,
              &params, TASK_PRIO_SENSOR_SERVICE, NULL);
  ESP_LOGI(TAG, "Fast sensor service started.");
  return ESP_OK;
}
