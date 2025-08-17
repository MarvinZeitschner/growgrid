#include "hal_i2c.h"
#include "i2cdev.h"

esp_err_t hal_i2c_init(void) {
    return i2cdev_init();
}
