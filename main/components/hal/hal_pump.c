#include "hal_pump.h"
#include "board.h"
#include "driver/gpio.h"
#include "event_bus.h"

esp_err_t hal_pump_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PUMP_RELAY_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    return gpio_config(&io_conf);
}

esp_err_t hal_pump_on(void) {
    esp_err_t err = gpio_set_level(PUMP_RELAY_GPIO, 1);
    if (err == ESP_OK) {
        event_t event = {
            .type = EVENT_TYPE_PUMP_STATE_CHANGE,
            .data.pump_state.is_on = true
        };
        event_bus_post(&event, 0);
    }
    return err;
}

esp_err_t hal_pump_off(void) {
    esp_err_t err = gpio_set_level(PUMP_RELAY_GPIO, 0);
    if (err == ESP_OK) {
        event_t event = {
            .type = EVENT_TYPE_PUMP_STATE_CHANGE,
            .data.pump_state.is_on = false
        };
        event_bus_post(&event, 0);
    }
    return err;
}
