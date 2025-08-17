#ifndef BOARD_H
#define BOARD_H

#include "soc/gpio_num.h"

#define LED_RGB_GPIO GPIO_NUM_8

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN GPIO_NUM_6
#define I2C_SCL_PIN GPIO_NUM_7
#define I2C_FREQ_HZ 400000

#define SOIL_ADC_CHANNEL ADC_CHANNEL_0
#define SOIL_OUT_MAX 2200
#define SOIL_OUT_MIN 900

#define PUMP_RELAY_GPIO GPIO_NUM_10

#endif
