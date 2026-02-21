#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// RENOMBRADO: nvs_wifi_config_t para no chocar con ESP-IDF
typedef struct {
    char ssid[32];
    char password[64];
} nvs_wifi_config_t;

typedef struct {
    char url[128];
    int method; // 0=GET, 1=POST
    char payload[128];
    int timeout_ms;
} button_config_t;

typedef struct {
    char user[32];
    char pass[64];
} admin_config_t;

void app_nvs_init(void);
esp_err_t app_nvs_save_wifi(const char* ssid, const char* pass);
bool app_nvs_get_wifi_config(nvs_wifi_config_t *config);
esp_err_t app_nvs_save_button(int btn_id, button_config_t *config);
esp_err_t app_nvs_get_button_config(int btn_id, button_config_t *config);
esp_err_t app_nvs_save_admin(const char* user, const char* pass);
void app_nvs_get_admin(admin_config_t *config);
void app_nvs_clear_all(void);
