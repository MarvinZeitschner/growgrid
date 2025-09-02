#include "platform_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "event_bus.h"
#include "freertos/event_groups.h"
#include <math.h>
#include <string.h>

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MAX_RETRIES 15
#define INITIAL_RETRY_DELAY_MS 1000
#define MAX_RETRY_DELAY_MS 60000

static const char *TAG = "PLATFORM_WIFI";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    event_t event = {.type = EVENT_TYPE_WIFI_DISCONNECTED};
    event_bus_post(&event, 0);
    if (s_retry_num < MAX_RETRIES) {
      uint32_t delay_ms =
          (uint32_t)(INITIAL_RETRY_DELAY_MS * pow(2, s_retry_num));
      if (delay_ms > MAX_RETRY_DELAY_MS) {
        delay_ms = MAX_RETRY_DELAY_MS;
      }
      // ESP_LOGI(TAG, "Retry %d/%d: Disconnected. Reconnecting in %dms...",
      // s_retry_num + 1, MAX_RETRIES, delay_ms);
      vTaskDelay(pdMS_TO_TICKS(delay_ms));
      esp_wifi_connect();
      s_retry_num++;
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      ESP_LOGE(TAG, "Failed to connect to the AP after %d retries.",
               MAX_RETRIES);
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *ip_event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP address:" IPSTR, IP2STR(&ip_event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    event_t event = {.type = EVENT_TYPE_WIFI_CONNECTED};
    event_bus_post(&event, 0);
  }
}

esp_err_t platform_wifi_init_sta(const char *ssid, const char *pass) {
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // Disable Wi-Fi power save mode
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  wifi_config_t wifi_config = {
      .sta =
          {
              .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
              .pmf_cfg = {.capable = true, .required = false},
          },
  };
  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, pass,
          sizeof(wifi_config.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "platform_wifi_init_sta finished. Waiting for connection...");

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Connected to AP SSID: %s", ssid);
    return ESP_OK;
  } else {
    ESP_LOGE(TAG, "Failed to connect to SSID: %s", ssid);
    return ESP_FAIL;
  }
}
