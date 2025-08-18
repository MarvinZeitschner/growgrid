#include "provisioning.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "storage.h"

static const char *TAG = "PROVISIONING";

static int hex_to_int(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

static void url_decode(char *dest, const char *src) {
  char *p = dest;
  while (*src) {
    if (*src == '+') {
      *p++ = ' ';
    } else if (*src == '%' && src[1] && src[2]) {
      int high = hex_to_int(src[1]);
      int low = hex_to_int(src[2]);
      if (high != -1 && low != -1) {
        *p++ = (char)(high * 16 + low);
        src += 2;
      } else {
        *p++ = *src;
      }
    } else {
      *p++ = *src;
    }
    src++;
  }
  *p = '\0';
}

static void start_webserver(void);

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t *event =
        (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac),
             event->aid);
  } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t *event =
        (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d", MAC2STR(event->mac),
             event->aid);
  }
}

static void wifi_init_softap(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
      .ap = {.ssid = "esp32c6",
             .ssid_len = strlen("esp32c6"),
             .password = "123456789",
             .max_connection = 1,
             .authmode = WIFI_AUTH_WPA_WPA2_PSK},
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_softap finished. SSID: %s password: %s", "esp32c6",
           "growgrid");
}

static esp_err_t save_post_handler(httpd_req_t *req) {
  char buf[512];
  int ret, remaining = req->content_len;

  if (remaining > sizeof(buf) - 1) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
    return ESP_FAIL;
  }

  ret = httpd_req_recv(req, buf, remaining);
  if (ret <= 0) {
    return ESP_FAIL;
  }
  buf[ret] = '\0';

  credentials_t creds;
  memset(&creds, 0, sizeof(credentials_t));

  char param_buf[128];
  if (httpd_query_key_value(buf, "ssid", param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    url_decode(creds.wifi_ssid, param_buf);
  }
  if (httpd_query_key_value(buf, "pass", param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    url_decode(creds.wifi_pass, param_buf);
  }
  if (httpd_query_key_value(buf, "mqtt_broker", param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    url_decode(creds.mqtt_broker, param_buf);
  }
  if (httpd_query_key_value(buf, "mqtt_user", param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    url_decode(creds.mqtt_user, param_buf);
  }
  if (httpd_query_key_value(buf, "mqtt_pass", param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    url_decode(creds.mqtt_pass, param_buf);
  }
  if (httpd_query_key_value(buf, "ntp_server", param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    url_decode(creds.ntp_server, param_buf);
  }

  storage_save_credentials(&creds);

  const char *resp_str = "<html><body><h1>Credentials Saved!</h1><p>Device "
                         "will now restart.</p></body></html>";
  httpd_resp_send(req, resp_str, strlen(resp_str));

  vTaskDelay(pdMS_TO_TICKS(5000));
  esp_restart();

  return ESP_OK;
}

static esp_err_t root_get_handler(httpd_req_t *req) {
  const char *resp_str =
      "<html><head><title>GrowGrid "
      "Setup</"
      "title><style>body{font-family:sans-serif;background-color:#f0f0f0;}."
      "container{background-color:white;margin:50px "
      "auto;padding:20px;border-radius:8px;max-width:500px;box-shadow:0 2px "
      "4px "
      "rgba(0,0,0,0.1);}h1{color:#333;}input{width:100%;padding:8px;margin:"
      "10px 0;box-sizing:border-box;border:1px solid "
      "#ccc;border-radius:4px;}button{background-color:#4CAF50;color:white;"
      "padding:14px 20px;margin:8px "
      "0;border:none;cursor:pointer;width:100%;border-radius:4px;}button:hover{"
      "opacity:0.8;}</style></head><body><div class='container'><h1>GrowGrid "
      "Wi-Fi & MQTT Setup</h1><form action='/save' method='post'><h2>Wi-Fi "
      "Settings</h2><input type='text' name='ssid' placeholder='WiFi SSID' "
      "required><br><input type='password' name='pass' placeholder='WiFi "
      "Password'><br><h2>MQTT Settings</h2><input type='text' "
      "name='mqtt_broker' placeholder='mqtt://broker.address:1883' "
      "required><br><input type='text' name='mqtt_user' placeholder='MQTT "
      "Username'><br><input type='password' name='mqtt_pass' placeholder='MQTT "
      "Password'><br><h2>NTP Settings</h2><input type='text' name='ntp_server' "
      "placeholder='pool.ntp.org' required><br><button type='submit'>Save and "
      "Restart</button></form></div></body></html>";
  httpd_resp_send(req, resp_str, strlen(resp_str));
  return ESP_OK;
}

static void start_webserver(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
    };
    httpd_register_uri_handler(server, &root_uri);

    httpd_uri_t save_uri = {
        .uri = "/save",
        .method = HTTP_POST,
        .handler = save_post_handler,
    };
    httpd_register_uri_handler(server, &save_uri);
  }
}

esp_err_t provisioning_start(void) {
  ESP_LOGI(TAG, "Starting provisioning mode");
  wifi_init_softap();
  start_webserver();
  return ESP_OK;
}
