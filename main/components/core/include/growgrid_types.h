#pragma once
#include <stdint.h>

typedef enum {
  SENSOR_DATA_TYPE_TEMP_HUMIDITY,
  SENSOR_DATA_TYPE_LIGHT,
  SENSOR_DATA_TYPE_SOIL_MOISTURE,
} sensor_data_type_t;

typedef struct {
  float temperature;
  float humidity;
} temp_humidity_data_t;

typedef struct {
  uint32_t lux;
} light_data_t;

typedef struct {
  int percent;
} soil_moisture_data_t;

typedef union {
  temp_humidity_data_t temp_humidity;
  light_data_t light;
  soil_moisture_data_t soil_moisture;
} sensor_data_payload_t;

typedef struct {
  uint64_t timestamp_us;
  sensor_data_type_t type;
  sensor_data_payload_t payload;
} sensor_data_t;
