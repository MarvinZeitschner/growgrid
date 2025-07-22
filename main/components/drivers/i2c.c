#include "i2c.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/i2c_types.h"
#include "soc/clk_tree_defs.h"
#include <driver/i2c_master.h>
#include <stdint.h>

static const char *TAG = "I2C";

esp_err_t i2c_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl,
                   i2c_master_bus_handle_t *bus_handle) {
  ESP_LOGI(TAG, "Initializing I2C");

  i2c_master_bus_config_t i2c_master_conf = {.clk_source = I2C_CLK_SRC_DEFAULT,
                                             .i2c_port = port,
                                             .sda_io_num = sda,
                                             .scl_io_num = scl,
                                             .glitch_ignore_cnt = 7,
                                             .flags.enable_internal_pullup =
                                                 true};

  return i2c_new_master_bus(&i2c_master_conf, bus_handle);
}

esp_err_t i2c_add_device(i2c_master_bus_handle_t bus_handle,
                         uint8_t device_address, uint32_t freq,
                         i2c_master_dev_handle_t *dev_handle) {
  ESP_LOGI(TAG, "Adding I2C device");

  i2c_device_config_t dev_conf = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = device_address,
      .scl_speed_hz = freq,
  };

  return i2c_master_bus_add_device(bus_handle, &dev_conf, dev_handle);
}
