idf_component_register(
  SRCS
  "rgb_led.c"
  INCLUDE_DIRS
  "include"
  REQUIRES
  board
  led_strip)
