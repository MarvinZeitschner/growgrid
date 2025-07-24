#include "tsl2561.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c_bus.h"
#include <stdint.h>

static const char *TAG = "TSL2561";

tsl2561_handle_t tsl2561_create(i2c_bus_handle_t bus, uint8_t dev_addr) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)calloc(1, sizeof(tsl2561_dev_t));
  sens->i2c_dev =
      i2c_bus_device_create(bus, dev_addr, i2c_bus_get_current_clk_speed(bus));
  if (sens->i2c_dev == NULL) {
    ESP_LOGE(TAG, "Error creating tsl2561: i2c_dev is NULL");
    free(sens);
    return NULL;
  }
  sens->dev_addr = dev_addr;
  return (tsl2561_handle_t)sens;
}

esp_err_t tsl2561_delete(tsl2561_handle_t *sensor) {
  if (*sensor == NULL) {
    return ESP_OK;
  }
  tsl2561_dev_t *sens = (tsl2561_dev_t *)(*sensor);
  i2c_bus_device_delete(&sens->i2c_dev);
  free(sens);
  *sensor = NULL;
  return ESP_OK;
}

esp_err_t tsl2561_default_init(tsl2561_handle_t sensor) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)sensor;

  sens->integration_time = TSL2561_INTEGRATION_402;
  sens->gain = TSL2561_GAIN_1x;

  uint8_t timing = sens->gain | (sens->integration_time & 0x03);
  esp_err_t err = i2c_bus_write_byte(sens->i2c_dev,
                                     TSL2561_CMD | TSL2561_REG_TIMING, timing);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing tsl2561");
    return err;
  }

  return tsl2561_power_on(sensor);
}

esp_err_t tsl2561_power_on(tsl2561_handle_t sensor) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)sensor;

  uint8_t data = TSL2561_CMD_POWER_ON;
  return i2c_bus_write_byte(sens->i2c_dev, TSL2561_CMD | TSL2561_REG_CONTROL,
                            data);
}

esp_err_t tsl2561_power_off(tsl2561_handle_t sensor) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)sensor;

  uint8_t data = TSL2561_CMD_POWER_OFF;
  return i2c_bus_write_byte(sens->i2c_dev, TSL2561_CMD | TSL2561_REG_CONTROL,
                            data);
}

esp_err_t tsl2561_set_config(tsl2561_handle_t sensor,
                             const tsl2561_config_t config) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)sensor;

  sens->integration_time = config.integration_time;
  sens->gain = config.gain;

  uint8_t timing = sens->gain | (sens->integration_time & 0x03);
  return i2c_bus_write_byte(sens->i2c_dev, TSL2561_CMD | TSL2561_REG_TIMING,
                            timing);
}

esp_err_t tsl2561_read_channels(tsl2561_handle_t sensor, uint16_t *ch0,
                                uint16_t *ch1) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)sensor;

  uint8_t reg = TSL2561_CMD_WORD | TSL2561_REG_DATA0LOW;
  uint8_t data0[2];
  esp_err_t err = i2c_bus_read_bytes(sens->i2c_dev, reg, 2, data0);
  *ch0 = (data0[1] << 8) | data0[0];

  reg = TSL2561_CMD_WORD | TSL2561_REG_DATA1LOW;
  uint8_t data1[2];
  err = i2c_bus_read_bytes(sens->i2c_dev, reg, 2, data1);
  if (err != ESP_OK)
    return err;
  *ch1 = (data1[1] << 8) | data1[0];

  return ESP_OK;
}

esp_err_t tsl2561_read_lux(tsl2561_handle_t sensor, uint32_t *lux) {
  tsl2561_dev_t *sens = (tsl2561_dev_t *)sensor;

  uint16_t ch0 = 0, ch1 = 0;
  esp_err_t err = tsl2561_read_channels(sensor, &ch0, &ch1);
  if (err != ESP_OK)
    return err;

  uint32_t ch_scale = 0;
  // first, scale the channel values depending on the gain and integration time
  // 16X, 402mS is nominal.
  // scale if integration time is NOT 402 msec
  switch (sens->integration_time) {
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
  if (sens->gain == TSL2561_GAIN_1x) {
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
