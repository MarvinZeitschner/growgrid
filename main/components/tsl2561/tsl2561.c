#include "tsl2561.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdint.h>

esp_err_t tsl2561_init(tsl2561_t *sensor, i2c_master_dev_handle_t dev_handle,
                       int timeout) {
  sensor->dev_handle = dev_handle;
  uint8_t cmd[2] = {TSL2561_COMMAND | TSL2561_REG_CONTROL, 0x03};
  return i2c_master_transmit(sensor->dev_handle, cmd, 2, timeout);
}

esp_err_t tsl2561_read_ch0(tsl2561_t *sensor, uint16_t *ch0, int timeout) {

  uint8_t reg = TSL2561_COMMAND | TSL2561_DATA0LOW;
  uint8_t data[2];
  esp_err_t err = i2c_master_transmit_receive(sensor->dev_handle, &reg, 1, data,
                                              2, timeout);
  if (err != ESP_OK)
    return err;

  *ch0 = (data[1] << 8 | data[0]);

  return ESP_OK;
}

esp_err_t tsl2561_power_off(tsl2561_t *sensor, int timeout) {
  uint8_t cmd[2] = {TSL2561_COMMAND | TSL2561_REG_CONTROL, 0x00};
  return i2c_master_transmit(sensor->dev_handle, cmd, 2, timeout);
}
