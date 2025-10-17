idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp32-camera esp_http_server freertos
                       PRIV_REQUIRES esp_wifi esp_netif nvs_flash)
