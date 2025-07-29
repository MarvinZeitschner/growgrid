#pragma once

#include <stdbool.h>

typedef struct {
  float temperature;
  int light;
  int soil_moisture;
} SensorData_t;

void sensor_data_init(SensorData_t *sensor_data);
