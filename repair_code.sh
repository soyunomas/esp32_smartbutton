#!/bin/bash

# Colores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}==> REPARANDO CÓDIGO FUENTE (DEBUGGING FINAL) <==${NC}"

# ==============================================================================
# 1. CORREGIR APP_NVS.H (Cambiar nombre de struct para evitar conflicto)
# ==============================================================================
echo -e "${GREEN}-> Corrigiendo app_nvs/include/app_nvs.h ...${NC}"
cat > components/app_nvs/include/app_nvs.h << 'EOF'
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

void app_nvs_init(void);
esp_err_t app_nvs_save_wifi(const char* ssid, const char* pass);
bool app_nvs_get_wifi_config(nvs_wifi_config_t *config);
esp_err_t app_nvs_save_button(int btn_id, button_config_t *config);
esp_err_t app_nvs_get_button_config(int btn_id, button_config_t *config);
void app_nvs_clear_all(void);
EOF

# ==============================================================================
# 2. CORREGIR APP_NVS.C (Actualizar referencias al nuevo struct)
# ==============================================================================
echo -e "${GREEN}-> Corrigiendo app_nvs/app_nvs.c ...${NC}"
cat > components/app_nvs/app_nvs.c << 'EOF'
#include "app_nvs.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS";

void app_nvs_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void app_nvs_clear_all(void) {
    nvs_flash_erase();
    app_nvs_init();
}

esp_err_t app_nvs_save_wifi(const char* ssid, const char* pass) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("wifi_conf", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    nvs_set_str(my_handle, "ssid", ssid);
    nvs_set_str(my_handle, "pass", pass);
    nvs_commit(my_handle);
    nvs_close(my_handle);
    return ESP_OK;
}

bool app_nvs_get_wifi_config(nvs_wifi_config_t *config) {
    nvs_handle_t my_handle;
    if (nvs_open("wifi_conf", NVS_READONLY, &my_handle) != ESP_OK) return false;

    size_t required_size = sizeof(config->ssid);
    if (nvs_get_str(my_handle, "ssid", config->ssid, &required_size) != ESP_OK) {
        nvs_close(my_handle);
        return false;
    }

    required_size = sizeof(config->password);
    if (nvs_get_str(my_handle, "pass", config->password, &required_size) != ESP_OK) {
        config->password[0] = 0; 
    }
    
    nvs_close(my_handle);
    return true;
}

esp_err_t app_nvs_save_button(int btn_id, button_config_t *config) {
    nvs_handle_t my_handle;
    char namespace[16];
    sprintf(namespace, "btn_%d", btn_id);
    
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    nvs_set_str(my_handle, "url", config->url);
    nvs_set_i32(my_handle, "method", config->method);
    nvs_set_str(my_handle, "payload", config->payload);
    nvs_commit(my_handle);
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t app_nvs_get_button_config(int btn_id, button_config_t *config) {
    nvs_handle_t my_handle;
    char namespace[16];
    sprintf(namespace, "btn_%d", btn_id);

    if (nvs_open(namespace, NVS_READONLY, &my_handle) != ESP_OK) return ESP_FAIL;

    size_t size = sizeof(config->url);
    nvs_get_str(my_handle, "url", config->url, &size);
    
    int32_t method = 0;
    nvs_get_i32(my_handle, "method", &method);
    config->method = method;

    size = sizeof(config->payload);
    if(nvs_get_str(my_handle, "payload", config->payload, &size) != ESP_OK) {
        config->payload[0] = 0;
    }

    nvs_close(my_handle);
    return ESP_OK;
}
EOF

# ==============================================================================
# 3. CORREGIR APP_HTTP.C (Quitar crt_bundle y usar nuevo struct)
# ==============================================================================
echo -e "${GREEN}-> Corrigiendo app_http/app_http.c ...${NC}"
cat > components/app_http/app_http.c << 'EOF'
#include "app_http.h"
#include "app_nvs.h"
#include "app_core.h"
#include "app_led.h"
#include "esp_http_client.h"
#include "esp_log.h"

// NOTA: Se ha quitado esp_crt_bundle.h para facilitar compilación inicial

static const char *TAG = "HTTP";

void http_execute_task(void *pvParameters) {
    int btn_id = (int)pvParameters;
    button_config_t config;
    
    if(app_nvs_get_button_config(btn_id, &config) != ESP_OK) {
        ESP_LOGE(TAG, "No config for btn %d", btn_id);
        vTaskDelete(NULL);
        return;
    }

    if (strlen(config.url) < 5) {
        ESP_LOGE(TAG, "Invalid URL");
        vTaskDelete(NULL);
        return;
    }

    esp_http_client_config_t http_config = {
        .url = config.url,
        .timeout_ms = config.timeout_ms > 0 ? config.timeout_ms : 5000,
        .method = (config.method == 1) ? HTTP_METHOD_POST : HTTP_METHOD_GET,
        // .crt_bundle_attach = esp_crt_bundle_attach, // Deshabilitado temporalmente
    };

    esp_http_client_handle_t client = esp_http_client_init(&http_config);

    if (config.method == 1 && strlen(config.payload) > 0) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, config.payload, strlen(config.payload));
    }

    app_set_state(STATE_HTTP_REQ);
    
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Status = %d, content_length = %lld", status, esp_http_client_get_content_length(client));
        if (status >= 200 && status < 300) {
            app_led_signal_success();
        } else {
            app_led_signal_error();
        }
    } else {
        ESP_LOGE(TAG, "HTTP Request failed: %s", esp_err_to_name(err));
        app_led_signal_error();
    }

    esp_http_client_cleanup(client);
    app_set_state(STATE_NORMAL);
    vTaskDelete(NULL);
}

void app_http_trigger(int btn_id) {
    xTaskCreate(http_execute_task, "http_req", 8192, (void*)btn_id, 5, NULL);
}
EOF

# ==============================================================================
# 4. CORREGIR APP_WIFI.C (Arreglar colisión de nombres y Designated Initializers)
# ==============================================================================
echo -e "${GREEN}-> Corrigiendo app_wifi/app_wifi.c ...${NC}"
cat > components/app_wifi/app_wifi.c << 'EOF'
#include "app_wifi.h"
#include "app_core.h"
#include "app_nvs.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "WIFI";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (app_get_state() != STATE_AP_MODE) {
            ESP_LOGW(TAG, "WiFi Lost, retrying...");
            esp_wifi_connect();
            xEventGroupSetBits(app_event_group, EVENT_WIFI_LOST);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi Connected!");
        xEventGroupSetBits(app_event_group, EVENT_WIFI_CONNECTED);
        app_set_state(STATE_NORMAL);
    }
}

void app_wifi_start_sta(void) {
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    // Usamos el struct renombrado de NVS
    nvs_wifi_config_t nvs_conf;
    if (!app_nvs_get_wifi_config(&nvs_conf)) {
        ESP_LOGE(TAG, "No WiFi config found in NVS");
        return;
    }

    // Configuración de ESP-IDF
    wifi_config_t wifi_config = {0};
    
    // Copia segura de strings
    strlcpy((char*)wifi_config.sta.ssid, nvs_conf.ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, nvs_conf.password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
}

void app_wifi_start_ap(void) {
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    char ssid[32];
    sprintf(ssid, "SmartButton-%02X%02X", mac[4], mac[5]);

    wifi_config_t wifi_config = {0};
    
    // Configuración manual para evitar error de designated initializers anidados
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    
    strlcpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.ssid_len = strlen(ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "AP Started. SSID: %s", ssid);
}
EOF

# ==============================================================================
# 5. CORREGIR MAIN.C (Actualizar referencia a struct renombrado)
# ==============================================================================
echo -e "${GREEN}-> Corrigiendo main/main.c ...${NC}"
cat > main/main.c << 'EOF'
#include "app_core.h"
#include "app_nvs.h"
#include "app_wifi.h"
#include "app_web.h"
#include "app_buttons.h"
#include "app_led.h"
#include "esp_event.h"
#include "esp_log.h"

void app_main(void) {
    // Inicialización de sistema
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    app_nvs_init();
    app_event_group = xEventGroupCreate();
    app_led_init();
    app_buttons_init();

    // Lógica de arranque
    nvs_wifi_config_t conf;
    bool configured = app_nvs_get_wifi_config(&conf);

    if (!configured) {
        app_set_state(STATE_NO_CONFIG);
        app_wifi_start_ap();
        app_web_start();
    } else {
        app_set_state(STATE_CONNECTING);
        app_wifi_start_sta();
        app_web_start();
    }

    // Loop principal (Watchdog lógico)
    while (1) {
        if (app_get_state() == STATE_FACTORY_RESET) {
            app_led_set_blink(50, 50);
            ESP_LOGW("MAIN", "ERASING NVS...");
            app_nvs_clear_all();
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
EOF

echo -e "${BLUE}==> ¡REPARACIÓN COMPLETADA!${NC}"
echo "Ejecuta ahora:"
echo "1. idf.py build"
