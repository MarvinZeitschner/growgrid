#include "soil_sensor_service.h"
#include "app_config.h"
#include "board.h"
#include "esp_log.h"
// #include "esp_task_wdt.h"
#include "sensor_aggregator.h"
#include "soil_sensor.h"

static const char *TAG = "SOIL_SERVICE";

static void read_soil_moisture_task(void *pvParameter) {
  QueueHandle_t data_queue = (QueueHandle_t)pvParameter;

  adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1};
  adc_oneshot_chan_cfg_t ch_config = {.bitwidth = ADC_BITWIDTH_DEFAULT,
                                      .atten = ADC_ATTEN_DB_12};
  soil_sensor_config_t s_conf = {.adc_pin = GPIO_NUM_0,
                                 .init_config = init_config,
                                 .channel_config = ch_config};
  soil_sensor_handle_t soil_handle = soil_sensor_create(s_conf);
  soil_sensor_set_calibration(soil_handle, SOIL_OUT_MAX, SOIL_OUT_MIN);

  // ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  sensor_data_t data = {.type = DATA_TYPE_SOIL_MOISTURE};

  while (1) {
    if (soil_sensor_read_percent(soil_handle, &data.i_value) == ESP_OK) {
      xQueueSend(data_queue, &data, portMAX_DELAY);
    }
    // esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(SENSOR_POLLING_INTERVAL_MS));
  }
}

esp_err_t soil_sensor_service_start(QueueHandle_t data_queue) {
  xTaskCreate(&read_soil_moisture_task, "read_soil_moisture",
              TASK_STACK_SENSOR_SERVICE, (void *)data_queue,
              TASK_PRIO_SENSOR_SERVICE, NULL);
  ESP_LOGI(TAG, "Soil sensor service started.");
  return ESP_OK;
}
