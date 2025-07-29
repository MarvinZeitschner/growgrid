idf_component_register(
  SRCS "power_manager.c"
  INCLUDE_DIRS "include"
  REQUIRES
  app_config
  sensor_data
  mqtt_manager
  wifi_manager
  esp_wifi
  soil_sensor_service
)