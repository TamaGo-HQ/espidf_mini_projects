#include "esp_camera.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#define TAG "camera"

// AI-Thinker ESP32-CAM pin configuration
static camera_config_t camera_config = {
    .pin_pwdn = 32,
    .pin_reset = -1,
    .pin_xclk = 0,
    .pin_sccb_sda = 26,
    .pin_sccb_scl = 27,
    .pin_d7 = 35,
    .pin_d6 = 34,
    .pin_d5 = 39,
    .pin_d4 = 36,
    .pin_d3 = 21,
    .pin_d2 = 19,
    .pin_d1 = 18,
    .pin_d0 = 5,
    .pin_vsync = 25,
    .pin_href = 23,
    .pin_pclk = 22,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 63, // lower quality -> faster encoding
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// ---------------- Soft AP ----------------
static void wifi_init_softap(void) {
  esp_netif_create_default_wifi_ap();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {
      .ap = {.ssid = "ESP32-CAM",
             .ssid_len = strlen("ESP32-CAM"),
             .channel = 1,
             .max_connection = 4,
             .authmode = WIFI_AUTH_OPEN},
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "AP started: ESP32-CAM");
}

// --------------- MJPEG handler ----------------
static esp_err_t stream_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");

  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      ESP_LOGW(TAG, "Camera capture failed");
      vTaskDelay(pdMS_TO_TICKS(3000)); // wait before retrying
      continue;
    }

    // Prepare header
    char part_header[128];
    int hlen = snprintf(
        part_header, sizeof(part_header),
        "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n",
        fb->len);

    // Send header
    if (httpd_resp_send_chunk(req, part_header, hlen) != ESP_OK) {
      ESP_LOGW(TAG, "Failed to send header");
      esp_camera_fb_return(fb);
      break;
    }

    // Send image in one chunk
    if (httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK) {
      ESP_LOGW(TAG, "Failed to send frame");
      esp_camera_fb_return(fb);
      break;
    }

    esp_camera_fb_return(fb); // return buffer

    vTaskDelay(pdMS_TO_TICKS(3000)); // wait 3s before next capture
  }

  return ESP_OK;
}

// --------------- Webserver ----------------
static httpd_handle_t start_webserver(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t stream_uri = {.uri = "/stream",
                              .method = HTTP_GET,
                              .handler = stream_handler,
                              .user_ctx = NULL};
    httpd_register_uri_handler(server, &stream_uri);
  }

  return server;
}

// --------------- Camera init ----------------
static esp_err_t init_camera(void) {
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Camera Init Failed");
    return err;
  }
  return ESP_OK;
}

// --------------- Main ----------------
void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_camera_set_psram_mode(true);

  wifi_init_softap();

  if (init_camera() != ESP_OK) {
    ESP_LOGE(TAG, "Camera init failed");
    return;
  }

  httpd_handle_t server = start_webserver();
  if (server == NULL) {
    ESP_LOGE(TAG, "Failed to start webserver");
  } else {
    ESP_LOGI(TAG, "Webserver started successfully");
  }
}
