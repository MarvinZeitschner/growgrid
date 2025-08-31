set(COMPONENT_EMBED_FILES
  "certs/ca.crt"
  "certs/esp32.crt"
  "certs/esp32.key"
)

idf_component_register(
  SRCS ""
  INCLUDE_DIRS "include"
  EMBED_FILES ${COMPONENT_EMBED_FILES}
)
