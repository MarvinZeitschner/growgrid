#pragma once

#include "portmacro.h"
#include <stdbool.h>

typedef struct {
  float temperature;
  float light;
  float soil_moisture;

  TickType_t temp_light_timestamp;
  TickType_t soil_moisture_timestamp;
} SensorData_t;

void sensor_data_init(SensorData_t *sensor_data);
