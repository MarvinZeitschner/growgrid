#include "fast_sensor_service.h"
#include "app_config.h"
#include "bmp280.h"
#include "board.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "sensor_data.h"
#include "tsl2561.h"
#include <stdint.h>
#include <string.h>

static const char *TAG = "FAST_SENSOR_SERVICE";

typedef struct {
  EventGroupHandle_t event_group;
  SemaphoreHandle_t data_mutex;
  SensorData_t *shared_sensor_data;
} task_params_t;

static void fast_sensor_task(void *pvParameter) {
  task_params_t config = *(task_params_t *)pvParameter;

  bmp280_t bmp280 = {0};
  bmp280_params_t params = {.mode = BMP280_MODE_NORMAL,
                            .filter = BMP280_FILTER_OFF,
                            .oversampling_pressure = BMP280_SKIPPED,
                            .oversampling_temperature = BMP280_ULTRA_HIGH_RES,
                            .oversampling_humidity = BMP280_SKIPPED,
                            .standby = BMP280_STANDBY_250};

  ESP_ERROR_CHECK(bmp280_init_desc(&bmp280, BMP280_I2C_ADDRESS_0, 0,
                                   I2C_SDA_PIN, I2C_SCL_PIN));
  ESP_ERROR_CHECK(bmp280_init(&bmp280, &params));

  tsl2561_t tsl2561 = {0};
  ESP_ERROR_CHECK(tsl2561_init_desc(&tsl2561, TSL2561_I2C_ADDR_FLOAT, 0,
                                    I2C_SDA_PIN, I2C_SCL_PIN));
  tsl2561_set_integration_time(&tsl2561, TSL2561_INTEGRATION_402MS);
  ESP_ERROR_CHECK(tsl2561_init(&tsl2561));

  ESP_LOGI(TAG, "Initialized BMP280 + TSL2561");

  float temp = -999.f;
  uint32_t lux = -1;
  float pressure;

  TickType_t last_wake_time = xTaskGetTickCount();

  while (1) {
    esp_err_t temp_res = bmp280_read_float(&bmp280, &temp, &pressure, NULL);
    esp_err_t lux_res = tsl2561_read_lux(&tsl2561, &lux);

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

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

esp_err_t fast_sensor_service_start(EventGroupHandle_t event_group,
                                    SemaphoreHandle_t data_mutex,
                                    SensorData_t *shared_sensor_data) {
  static task_params_t params;

  params.event_group = event_group;
  params.data_mutex = data_mutex;
  params.shared_sensor_data = shared_sensor_data;

  xTaskCreate(&fast_sensor_task, "fast_sensors", TASK_STACK_SENSOR_SERVICE,
              &params, TASK_PRIO_SENSOR_SERVICE, NULL);
  ESP_LOGI(TAG, "Fast sensor service started.");
  return ESP_OK;
}
