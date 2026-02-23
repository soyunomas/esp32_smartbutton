#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Estructura WiFi
typedef struct {
    char ssid[32];
    char password[64];
} nvs_wifi_config_t;

// Estructura Botón
typedef struct {
    char url[512];
    int method; // 0=GET, 1=POST
    char payload[128];
    int timeout_ms;
    int cooldown_ms; 
} button_config_t;

// Estructura Admin & Sistema (Ampliando configuraciones de AP)
typedef struct {
    char user[32];
    char pass[64];
    int reset_time_ms;
    char ap_ssid[32];
    char ap_pass[64];
    bool pure_client;
    bool deep_sleep;
    int sta_max_retries;
    int ap_channel;
    int wakeup_timeout_s;
    int config_awake_s;
    int debounce_ms;
} admin_config_t;

void app_nvs_init(void);
void app_nvs_clear_all(void);

// Funciones WiFi
esp_err_t app_nvs_save_wifi(const char* ssid, const char* pass);
bool app_nvs_get_wifi_config(nvs_wifi_config_t *config);

// Funciones Botones
esp_err_t app_nvs_save_button(int btn_id, button_config_t *config);
esp_err_t app_nvs_get_button_config(int btn_id, button_config_t *config);

// Funciones Admin
esp_err_t app_nvs_save_admin(const admin_config_t *config);
void app_nvs_get_admin(admin_config_t *config);
