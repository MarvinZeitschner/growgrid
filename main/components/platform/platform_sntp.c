#include "platform_sntp.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "secrets.h"
#include <time.h>

static const char *TAG = "SNTP";

static void time_sync_notification_cb(struct timeval *tv) {
  ESP_LOGI(TAG, "Time successfully synchronized");
}

void platform_sntp_init(void) {
  ESP_LOGI(TAG, "Initializing SNTP");
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, NTP_IP);
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  esp_sntp_init();

  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  const int retry_count = 15;

  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET &&
         ++retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry,
             retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

  if (retry == retry_count) {
    ESP_LOGE(TAG, "Failed to sync time");
  } else {
    time(&now);
    localtime_r(&now, &timeinfo);
  }
}
