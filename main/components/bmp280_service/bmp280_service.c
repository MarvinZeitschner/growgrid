#include "bmp280_service.h"
#include "app_config.h"
#include "bmp280.h"
#include "esp_log.h"
// #include "esp_task_wdt.h"
#include "freertos/task.h"
#include "sensor_aggregator.h"

static const char *TAG = "BMP280_SERVICE";

typedef struct {
  i2c_bus_handle_t i2c_handle;
  QueueHandle_t data_queue;
} task_params_t;

static void read_temperature_task(void *pvParameter) {
  task_params_t *p = (task_params_t *)pvParameter;
  i2c_bus_handle_t i2c_handle = p->i2c_handle;
  QueueHandle_t data_queue = p->data_queue;
  free(p);

  bmp280_handle_t bmp280_handle =
      bmp280_create(i2c_handle, BMP280_I2C_ADDRESS_DEFAULT);
  if (bmp280_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create BMP280 handle in task.");
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(bmp280_default_init(bmp280_handle));

  vTaskDelay(pdMS_TO_TICKS(100));

  // ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  sensor_data_t data = {.type = DATA_TYPE_TEMPERATURE};

  while (1) {
    bmp280_take_forced_measurement(bmp280_handle);
    if (bmp280_read_temperature(bmp280_handle, &data.f_value) == ESP_OK) {
      xQueueSend(data_queue, &data, portMAX_DELAY);
    }
    // esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(SENSOR_POLLING_INTERVAL_MS));
  }
}

esp_err_t bmp280_service_start(i2c_bus_handle_t i2c_handle,
                               QueueHandle_t data_queue) {
  task_params_t *params = (task_params_t *)malloc(sizeof(task_params_t));
  if (params == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for task parameters");
    return ESP_ERR_NO_MEM;
  }
  params->i2c_handle = i2c_handle;
  params->data_queue = data_queue;

  xTaskCreate(&read_temperature_task, "read_temperature",
              TASK_STACK_SENSOR_SERVICE, params, TASK_PRIO_SENSOR_SERVICE,
              NULL);
  ESP_LOGI(TAG, "BMP280 service started.");
  return ESP_OK;
}
