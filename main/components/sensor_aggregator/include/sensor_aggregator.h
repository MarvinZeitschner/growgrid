#pragma once

#include "freertos/FreeRTOS.h" // IWYU pragma: keep - required before other FreeRTOS headers
#include "freertos/queue.h"

typedef enum {
  DATA_TYPE_LUX,
  DATA_TYPE_TEMPERATURE,
  DATA_TYPE_SOIL_MOISTURE
} sensor_data_type_t;

typedef struct {
  sensor_data_type_t type;
  float value;
} sensor_data_t;
