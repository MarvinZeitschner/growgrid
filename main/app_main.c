#include "app_controller.h"
#include "esp_log.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "Starting GrowGrid application...");
  ESP_ERROR_CHECK(app_controller_init());
}
