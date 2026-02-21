idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES app_core app_nvs app_wifi app_http app_web app_buttons app_led app_dns nvs_flash driver esp_event)
