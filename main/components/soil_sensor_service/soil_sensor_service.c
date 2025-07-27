#include "soil_sensor_service.h"
#include "app_config.h"
#include "board.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "soil_sensor.h"
#include <stdlib.h>

static const char *TAG = "SOIL_SERVICE";

typedef struct {
  QueueHandle_t data_queue;
} task_params_t;

static void read_soil_moisture_task(void *pvParameter) {
  task_params_t *p = (task_params_t *)pvParameter;
  QueueHandle_t data_queue = p->data_queue;
  free(p);

  adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1};
  adc_oneshot_chan_cfg_t ch_config = {.bitwidth = ADC_BITWIDTH_DEFAULT,
                                      .atten = ADC_ATTEN_DB_12};
  soil_sensor_config_t s_conf = {.adc_pin = GPIO_NUM_0,
                                 .init_config = init_config,
                                 .channel_config = ch_config};
  soil_sensor_handle_t soil_handle = soil_sensor_create(s_conf);
  soil_sensor_set_calibration(soil_handle, SOIL_OUT_MAX, SOIL_OUT_MIN);

  int data = 0;

  TickType_t x_last_wake_time = xTaskGetTickCount();

  while (1) {
    if (soil_sensor_read_percent(soil_handle, &data) == ESP_OK) {
      xQueueOverwrite(data_queue, &data);
    }

    vTaskDelayUntil(&x_last_wake_time,
                    pdMS_TO_TICKS(SENSOR_POLLING_INTERVAL_MS));
  }
}

esp_err_t soil_sensor_service_start(QueueHandle_t data_queue) {
  task_params_t *params = (task_params_t *)malloc(sizeof(task_params_t));
  if (params == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for task parameters");
    return ESP_ERR_NO_MEM;
  }
  params->data_queue = data_queue;

  xTaskCreate(&read_soil_moisture_task, "read_soil_moisture",
              TASK_STACK_SENSOR_SERVICE, params, TASK_PRIO_SENSOR_SERVICE,
              NULL);
  ESP_LOGI(TAG, "Soil sensor service started.");
  return ESP_OK;
}
