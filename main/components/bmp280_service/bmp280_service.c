#include "bmp280_service.h"
#include "app_config.h"
#include "bmp280.h"
#include "board.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/queue.h"

static const char *TAG = "BMP280_SERVICE";

typedef struct {
  i2c_bus_handle_t i2c_handle;
  EventGroupHandle_t event_group;
  QueueHandle_t data_queue;
} task_params_t;

static void read_temperature_task(void *pvParameter) {
  task_params_t *p = (task_params_t *)pvParameter;
  i2c_bus_handle_t i2c_handle = p->i2c_handle;
  EventGroupHandle_t event_group = p->event_group;
  QueueHandle_t data_queue = p->data_queue;
  free(p);

  bmp280_handle_t bmp280_handle =
      bmp280_create(i2c_handle, BMP280_I2C_ADDRESS_DEFAULT);
  if (bmp280_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create BMP280 handle in task.");
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(bmp280_default_init(bmp280_handle));

  float data = 0.f;

  vTaskDelay(pdMS_TO_TICKS(100));

  // TickType_t x_last_wake_time = xTaskGetTickCount();

  // while (1) {
  if (bmp280_read_temperature(bmp280_handle, &data) == ESP_OK) {
    xQueueOverwrite(data_queue, &data);
    xEventGroupSetBits(event_group, EVENT_SENSOR_TEMP_BIT);
  }

  bmp280_delete(&bmp280_handle);
  vTaskDelete(NULL);
  // vTaskDelayUntil(&x_last_wake_time,
  // pdMS_TO_TICKS(SENSOR_POLLING_INTERVAL_MS));
  // }
}

esp_err_t bmp280_service_start(i2c_bus_handle_t i2c_handle,
                               QueueHandle_t data_queue,
                               EventGroupHandle_t event_group) {
  task_params_t *params = (task_params_t *)malloc(sizeof(task_params_t));
  if (params == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for task parameters");
    return ESP_ERR_NO_MEM;
  }
  params->i2c_handle = i2c_handle;
  params->event_group = event_group;
  params->data_queue = data_queue;

  xTaskCreate(&read_temperature_task, "read_temperature",
              TASK_STACK_SENSOR_SERVICE, params, TASK_PRIO_SENSOR_SERVICE,
              NULL);
  ESP_LOGI(TAG, "BMP280 service started.");
  return ESP_OK;
}
