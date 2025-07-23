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

esp_err_t tsl2561_read_lux(tsl2561_t *sensor, uint32_t *lux, int timeout) {
  uint16_t ch0 = 0, ch1 = 0;
  esp_err_t err = tsl2561_read_channels(sensor, &ch0, &ch1, timeout);
  if (err != ESP_OK)
    return err;

  uint32_t ch_scale = 0;
  // first, scale the channel values depending on the gain and integration time
  // 16X, 402mS is nominal.
  // scale if integration time is NOT 402 msec
  switch (sensor->integration_time) {
  case TSL2561_INTEGRATION_13:
    ch_scale = TSL2561_CHSCALE_TINT0;
    break;
  case TSL2561_INTEGRATION_101:
    ch_scale = TSL2561_CHSCALE_TINT1;
    break;
  default: // 402
    ch_scale = (1 << TSL2561_CH_SCALE);
    break;
  };

  // scale if gain is not 16x
  if (sensor->gain == TSL2561_GAIN_1x) {
    ch_scale = ch_scale << 4; // scale 1x to 16x
  }

  ch0 = (ch0 * ch_scale) >> TSL2561_CH_SCALE;
  ch1 = (ch1 * ch_scale) >> TSL2561_CH_SCALE;

  // find the ratio of the channel values (Channel1/Channel0)
  // protect against divide by zero
  uint32_t ratio1 = 0;
  if (ch0 != 0) {
    ratio1 = (ch1 << (TSL2561_RATIO_SCALE + 1)) / ch0;
  }
  // round the ratio value
  uint32_t ratio = (ratio1 + 1) >> 1;

  uint32_t b, m;
  if (ratio <= TSL2561_K1T) {
    b = TSL2561_B1T;
    m = TSL2561_M1T;
  } else if (ratio <= TSL2561_K2T) {
    b = TSL2561_B2T;
    m = TSL2561_M2T;
  } else if (ratio <= TSL2561_K3T) {
    b = TSL2561_B3T;
    m = TSL2561_M3T;
  } else if (ratio <= TSL2561_K4T) {
    b = TSL2561_B4T;
    m = TSL2561_M4T;
  } else if (ratio <= TSL2561_K5T) {
    b = TSL2561_B5T;
    m = TSL2561_M5T;
  } else if (ratio <= TSL2561_K6T) {
    b = TSL2561_B6T;
    m = TSL2561_M6T;
  } else if (ratio <= TSL2561_K7T) {
    b = TSL2561_B7T;
    m = TSL2561_M7T;
  } else {
    b = TSL2561_B8T;
    m = TSL2561_M8T;
  }

  int32_t temp = ((ch0 * b) - (ch1 * m));
  // Do not allow negative lux value
  if (temp < 0)
    temp = 0;
  // Round lsb (2^(LUX_SCALE-1))
  temp += (1 << (TSL2561_LUX_SCALE - 1));
  // Strip off fractional portion
  *lux = temp >> TSL2561_LUX_SCALE;

  return ESP_OK;
}
