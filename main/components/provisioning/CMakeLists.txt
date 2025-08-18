idf_component_register(SRCS "provisioning.c"
                      INCLUDE_DIRS "include"
                      REQUIRES esp_wifi esp_http_server storage esp_event)
