#include "tsl2561_service.h"
#include "app_config.h"
#include "board.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "tsl2561.h"
#include <stdint.h>

static const char *TAG = "TSL2561_SERVICE";

typedef struct {
  i2c_bus_handle_t i2c_handle;
  EventGroupHandle_t event_group;
  QueueHandle_t data_queue;
} task_params_t;

static void read_light_task(void *pvParameter) {
  task_params_t *p = (task_params_t *)pvParameter;
  i2c_bus_handle_t i2c_handle = p->i2c_handle;
  EventGroupHandle_t event_group = p->event_group;
  QueueHandle_t data_queue = p->data_queue;
  free(p);

  tsl2561_handle_t tsl_handle = tsl2561_create(i2c_handle, TSL2561_I2C_ADDR);
  if (tsl_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create TSL2561 handle in task.");
    vTaskDelete(NULL);
  }
  ESP_ERROR_CHECK(tsl2561_default_init(tsl_handle));

  int data = 0;

  // TickType_t x_last_wake_time = xTaskGetTickCount();

  // while (1) {
  if (tsl2561_read_lux(tsl_handle, &data) == ESP_OK) {
    xQueueOverwrite(data_queue, &data);
    xEventGroupSetBits(event_group, EVENT_SENSOR_LUX_BIT);
  }

  // vTaskDelayUntil(&x_last_wake_time,
  //                 pdMS_TO_TICKS(SENSOR_POLLING_INTERVAL_MS));
  // }

  tsl2561_delete(&tsl_handle);
  vTaskDelete(NULL);
}

esp_err_t tsl2561_service_start(i2c_bus_handle_t i2c_handle,
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

  xTaskCreate(&read_light_task, "read_light", TASK_STACK_SENSOR_SERVICE, params,
              TASK_PRIO_SENSOR_SERVICE, NULL);
  ESP_LOGI(TAG, "TSL2561 service started.");
  return ESP_OK;
}
