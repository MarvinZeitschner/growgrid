#include "soil_sensor_service.h"
#include "app_config.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "soil_sensor.h"
#include <stdint.h>
#include <stdlib.h>

static const char *TAG = "SOIL_SERVICE";

typedef struct {
  EventGroupHandle_t event_group;
  QueueHandle_t data_queue;
  SemaphoreHandle_t data_mutex;
  SensorData_t *shared_sensor_data;
} task_params_t;

static void read_soil_moisture_task(void *pvParameter) {
  task_params_t config = *(task_params_t *)pvParameter;

  adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1};
  adc_oneshot_chan_cfg_t ch_config = {.bitwidth = ADC_BITWIDTH_DEFAULT,
                                      .atten = ADC_ATTEN_DB_12};
  soil_sensor_config_t s_conf = {.adc_pin = GPIO_NUM_0,
                                 .init_config = init_config,
                                 .channel_config = ch_config,
                                 .sampling = SOIL_SAMPLING_X16};
  soil_sensor_handle_t soil_handle = soil_sensor_create(s_conf);
  if (soil_handle == NULL) {
    soil_sensor_delete(&soil_handle);
    ESP_LOGE(TAG, "Failed to create soil sensor handle in task.");
    vTaskDelete(NULL);
  }
  soil_sensor_set_calibration(soil_handle, SOIL_OUT_MAX, SOIL_OUT_MIN);

  while (1) {
    int soil = -1;

    esp_err_t soil_res = soil_sensor_read_percent(soil_handle, &soil);

    if (soil_res == ESP_OK) {
      if (xSemaphoreTake(config.data_mutex,
                         pdMS_TO_TICKS(SHARED_DATA_SEMAPHORE_TIMEOUT_MS)) ==
          pdTRUE) {
        config.shared_sensor_data->soil_moisture = soil;
        xEventGroupSetBits(config.event_group, EVENT_SENSOR_SOIL_BIT);

        xSemaphoreGive(config.data_mutex);
      } else {
        ESP_LOGE(TAG, "Failed to acquire mutex");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

esp_err_t soil_sensor_service_start(QueueHandle_t data_queue,
                                    EventGroupHandle_t event_group,
                                    SemaphoreHandle_t data_mutex,
                                    SensorData_t *shared_sensor_data) {
  static task_params_t params;
  params.event_group = event_group;
  params.data_queue = data_queue;
  params.data_mutex = data_mutex;
  params.shared_sensor_data = shared_sensor_data;

  xTaskCreate(&read_soil_moisture_task, "read_soil_moisture",
              TASK_STACK_SENSOR_SERVICE, &params, TASK_PRIO_SENSOR_SERVICE,
              NULL);
  ESP_LOGI(TAG, "Soil sensor service started.");
  return ESP_OK;
}
