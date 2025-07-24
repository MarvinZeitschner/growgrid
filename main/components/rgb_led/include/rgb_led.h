#ifndef RGB_LED_H
#define RGB_LED_H

#include "esp_err.h"
#include <stdint.h>

/**
 *  @brief Initializes onboard led-strip with GPIO8 with 10MHz resolution
 *
 *  @returns esp_err_t
 */
esp_err_t rgb_led__default_init(void);

/**
 *  @brief Set the color of the led-strip with RGB values
 *
 *  @param r: red value
 *  @param g: green value
 *  @param b: blue value
 *
 *  @return void
 */
esp_err_t rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b);

/**
 *  @brief Clears the led-strip
 *
 *  @return void
 */
esp_err_t rgb_led_clear(void);

#endif
