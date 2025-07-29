#include "sensor_data.h"

void sensor_data_init(SensorData_t *sensor_data) {
  sensor_data->light = -1;
  sensor_data->soil_moisture = -1;
  sensor_data->temperature = -999.f;
}
