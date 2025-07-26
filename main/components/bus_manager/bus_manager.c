#include "bus_manager.h"
#include "board.h"
#include "esp_log.h"

static const char *TAG = "BUS_MANAGER";
static i2c_bus_handle_t s_i2c_bus_handle = NULL;

esp_err_t bus_manager_init_i2c(void) {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_SDA_PIN,
      .scl_io_num = I2C_SCL_PIN,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_FREQ_HZ,
  };
  s_i2c_bus_handle = i2c_bus_create(I2C_PORT, &conf);
  if (s_i2c_bus_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create I2C bus handle.");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "I2C bus initialized successfully.");
  return ESP_OK;
}

i2c_bus_handle_t bus_manager_get_i2c_handle(void) { return s_i2c_bus_handle; }
