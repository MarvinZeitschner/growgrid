#include "tsl2561.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdint.h>

esp_err_t tsl2561_init(tsl2561_t *sensor, i2c_master_dev_handle_t dev_handle,
                       const tsl2561_config_t *config, int timeout) {
  sensor->dev_handle = dev_handle;
  sensor->integration_time = config->integration_time;
  sensor->gain = config->gain;

  uint8_t power_cmd[2] = {TSL2561_CMD | TSL2561_REG_CONTROL,
                          TSL2561_CMD_POWER_ON};
  esp_err_t err =
      i2c_master_transmit(sensor->dev_handle, power_cmd, 2, timeout);
  if (err != ESP_OK)
    return err;

  uint8_t timing = sensor->gain | (sensor->integration_time & 0x03);
  uint8_t timing_cmd[2] = {TSL2561_CMD | TSL2561_REG_TIMING, timing};
  return i2c_master_transmit(sensor->dev_handle, timing_cmd, 2, timeout);
}

esp_err_t tsl2561_power_off(tsl2561_t *sensor, int timeout) {
  uint8_t cmd[2] = {TSL2561_CMD | TSL2561_REG_CONTROL, TSL2561_CMD_POWER_OFF};
  return i2c_master_transmit(sensor->dev_handle, cmd, 2, timeout);
}

esp_err_t tsl2561_set_config(tsl2561_t *sensor, const tsl2561_config_t *config,
                             int timeout) {
  sensor->integration_time = config->integration_time;
  sensor->gain = config->gain;

  uint8_t timing = sensor->gain | (sensor->integration_time & 0x03);
  uint8_t timing_cmd[2] = {TSL2561_CMD | TSL2561_REG_TIMING, timing};
  return i2c_master_transmit(sensor->dev_handle, timing_cmd, 2, timeout);
}

esp_err_t tsl2561_read_channels(tsl2561_t *sensor, uint16_t *ch0, uint16_t *ch1,
                                int timeout) {
  uint8_t cmd0 = TSL2561_CMD_WORD | TSL2561_REG_DATA0LOW;
  uint8_t data0[2];
  esp_err_t err = i2c_master_transmit_receive(sensor->dev_handle, &cmd0, 1,
                                              data0, 2, timeout);
  if (err != ESP_OK)
    return err;
  *ch0 = (data0[1] << 8) | data0[0];

  uint8_t cmd1 = TSL2561_CMD_WORD | TSL2561_REG_DATA1LOW;
  uint8_t data1[2];
  err = i2c_master_transmit_receive(sensor->dev_handle, &cmd1, 1, data1, 2,
                                    timeout);
  if (err != ESP_OK)
    return err;
  *ch1 = (data1[1] << 8) | data1[0];

  return ESP_OK;
}
