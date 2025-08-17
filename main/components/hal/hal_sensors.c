#include "hal_sensors.h"
#include "board.h"
#include "bmp280.h"
#include "tsl2561.h"
#include "soil_sensor.h"
#include "esp_log.h"

static const char *TAG = "HAL_SENSORS";

// Device descriptors
static bmp280_t s_bmp280_dev;
static tsl2561_t s_tsl2561_dev;
static soil_sensor_handle_t s_soil_sensor_handle;

esp_err_t hal_sensors_init(void) {
    // Init BME280
    bmp280_params_t params = {
        .mode = BMP280_MODE_NORMAL,
        .filter = BMP280_FILTER_OFF,
        .oversampling_pressure = BMP280_SKIPPED,
        .oversampling_temperature = BMP280_STANDARD,
        .oversampling_humidity = BMP280_STANDARD,
        .standby = BMP280_STANDBY_250
    };
    ESP_ERROR_CHECK(bmp280_init_desc(&s_bmp280_dev, BMP280_I2C_ADDRESS_0, I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN));
    ESP_ERROR_CHECK(bmp280_init(&s_bmp280_dev, &params));

    // Init TSL2561
    ESP_ERROR_CHECK(tsl2561_init_desc(&s_tsl2561_dev, TSL2561_I2C_ADDR_FLOAT, I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN));
    ESP_ERROR_CHECK(tsl2561_set_integration_time(&s_tsl2561_dev, TSL2561_INTEGRATION_402MS));
    ESP_ERROR_CHECK(tsl2561_init(&s_tsl2561_dev));

    // Init Soil Sensor
    soil_sensor_config_t soil_cfg = {
        .adc_pin = SOIL_ADC_CHANNEL,
        .sampling = SOIL_SAMPLING_X16
    };
    s_soil_sensor_handle = soil_sensor_create(soil_cfg);
    if (s_soil_sensor_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create soil sensor");
        return ESP_FAIL;
    }
    soil_sensor_set_calibration(s_soil_sensor_handle, SOIL_OUT_MAX, SOIL_OUT_MIN);

    ESP_LOGI(TAG, "All sensors initialized");
    return ESP_OK;
}

esp_err_t hal_sensors_read_temp_humidity(temp_humidity_data_t *data) {
    float pressure; // Unused
    return bmp280_read_float(&s_bmp280_dev, &data->temperature, &pressure, &data->humidity);
}

esp_err_t hal_sensors_read_light(light_data_t *data) {
    uint32_t lux_val;
    esp_err_t err = tsl2561_read_lux(&s_tsl2561_dev, &lux_val);
    if (err == ESP_OK) {
        data->lux = lux_val;
    }
    return err;
}

esp_err_t hal_sensors_read_soil_moisture(soil_moisture_data_t *data) {
    return soil_sensor_read_percent(s_soil_sensor_handle, &data->percent);
}
