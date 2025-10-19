#include "esp_camera.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "credentials.h"

#define TAG "camera"
#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t s_wifi_event_group;

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
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 20, // lower quality -> faster encoding
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// --------------- Wi-Fi STA mode ----------------
// SSTA mode means STAtion mode. 
// your esp acts like a regular wifi client, just like your phone
// when it connects to a router

//callback reacts to wifi and ip events
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    //when wifi starts, connect
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    //if connections drops, reconnect and log 
    esp_wifi_connect();
    ESP_LOGI(TAG, "Retrying connection...");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    //when IP is obtained from router, log, set event bit
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "Connected! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

//wifi intialization & starting
static void wifi_init_sta(void) {

  //synchronizes WiFi connection Status
  s_wifi_event_group = xEventGroupCreate();

  //initializes network interface
  //ESP_ERROR_CHECK(esp_netif_init()); //already called in main
  esp_netif_create_default_wifi_sta();

  //intialize wifi driver
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  //register event hanler
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

  //set config  & mode to STA
  wifi_config_t wifi_config = 
  {
    .sta = 
    {
      .ssid = WIFI_SSID,
      .password = WIFI_PASS,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  
  //start WiFi
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "WiFi initialization finished.");

  // Wait (block) until connected
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Connected to SSID:%s", WIFI_SSID);
  } else {
    ESP_LOGE(TAG, "Failed to connect to SSID:%s", WIFI_SSID);
  }
}

// --------------- MJPEG handler ----------------
static esp_err_t stream_handler(httpd_req_t *req) {
  //multipart means the repsonse contains multiple parts, don't close 
  //i ll keep sending more data pieces as they come
  //x mixed replace, means new parts will replace the old ones
  //baoundary sperates each image
  // in short, it s telling browser this is an MJPEG stream
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
  if (server == NULL) {
    ESP_LOGE(TAG, "Failed to start webserver");
  } else {
    ESP_LOGI(TAG, "Webserver started successfully");
  }
  return server;
}

// --------------- Camera init ----------------
static esp_err_t init_camera(void) {
  esp_camera_set_psram_mode(true);
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

  wifi_init_sta();
  init_camera();
  start_webserver();
}
