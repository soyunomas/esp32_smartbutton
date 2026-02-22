idf_component_register(SRCS "app_web.c"
INCLUDE_DIRS "include"
REQUIRES esp_http_server app_wifi app_nvs app_core app_http app_led
PRIV_REQUIRES json esp_system mbedtls app_update spi_flash)
