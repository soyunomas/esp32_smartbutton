#include "app_wifi.h"
#include "app_core.h"
#include "app_nvs.h"
#include "app_dns.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "WIFI";
#define MAX_SCAN_RESULTS 15
#define STA_CONNECT_TIMEOUT_MS 15000

static int sta_retry_count = 0;
#define STA_MAX_RETRIES 5

static void start_ap_mode(void);

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        sta_retry_count++;
        if (sta_retry_count < STA_MAX_RETRIES) {
            ESP_LOGW(TAG, "WiFi Lost, retry %d/%d...", sta_retry_count, STA_MAX_RETRIES);
            esp_wifi_connect();
            xEventGroupSetBits(app_event_group, EVENT_WIFI_LOST);
        } else {
            ESP_LOGW(TAG, "STA failed after %d retries, switching to AP mode", STA_MAX_RETRIES);
            app_wifi_switch_to_ap();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        sta_retry_count = 0;
        ESP_LOGI(TAG, "WiFi Connected!");
        xEventGroupSetBits(app_event_group, EVENT_WIFI_CONNECTED);
        app_set_state(STATE_NORMAL);
    }
}

void app_wifi_start_sta(void) {
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    nvs_wifi_config_t nvs_conf;
    if (!app_nvs_get_wifi_config(&nvs_conf)) {
        ESP_LOGE(TAG, "No WiFi config found in NVS");
        return;
    }

    wifi_config_t wifi_config = {0};
    strlcpy((char*)wifi_config.sta.ssid, nvs_conf.ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, nvs_conf.password, sizeof(wifi_config.sta.password));

    sta_retry_count = 0;

    // Siempre APSTA: mantiene AP de configuración accesible
    start_ap_mode();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    app_dns_start();
    esp_wifi_connect();

    ESP_LOGI(TAG, "STA connecting to: %s (AP also active)", nvs_conf.ssid);
}

static void start_ap_mode(void) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    char ssid[32];
    sprintf(ssid, "SmartButton-%02X%02X", mac[4], mac[5]);

    wifi_config_t wifi_config = {0};
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    strlcpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.ssid_len = strlen(ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    ESP_LOGI(TAG, "AP Started. SSID: %s", ssid);
}

void app_wifi_start_ap(void) {
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    start_ap_mode();
    ESP_ERROR_CHECK(esp_wifi_start());

    app_set_state(STATE_AP_MODE);
}

void app_wifi_switch_to_ap(void) {
    ESP_LOGW(TAG, "Switching to AP mode (fallback)");
    esp_wifi_disconnect();
    start_ap_mode();
    app_dns_start();
    app_set_state(STATE_AP_MODE);
}

cJSON *app_wifi_scan(void) {
    wifi_scan_config_t scan_config = {
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };

    esp_wifi_scan_start(&scan_config, true);

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    if (ap_count > MAX_SCAN_RESULTS) ap_count = MAX_SCAN_RESULTS;

    wifi_ap_record_t *ap_list = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (!ap_list) return cJSON_CreateArray();

    esp_wifi_scan_get_ap_records(&ap_count, ap_list);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < ap_count; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "ssid", (char *)ap_list[i].ssid);
        cJSON_AddNumberToObject(item, "rssi", ap_list[i].rssi);
        cJSON_AddNumberToObject(item, "auth", ap_list[i].authmode);
        cJSON_AddItemToArray(arr, item);
    }

    free(ap_list);
    return arr;
}

cJSON *app_wifi_get_netinfo(void) {
    cJSON *root = cJSON_CreateObject();
    char buf[64];

    // Info STA
    esp_netif_t *sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(sta, &ip_info) == ESP_OK) {
            sprintf(buf, IPSTR, IP2STR(&ip_info.ip));
            cJSON_AddStringToObject(root, "ip", buf);
            sprintf(buf, IPSTR, IP2STR(&ip_info.netmask));
            cJSON_AddStringToObject(root, "mask", buf);
            sprintf(buf, IPSTR, IP2STR(&ip_info.gw));
            cJSON_AddStringToObject(root, "gw", buf);
        }
        esp_netif_dns_info_t dns;
        if (esp_netif_get_dns_info(sta, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK) {
            sprintf(buf, IPSTR, IP2STR(&dns.ip.u_addr.ip4));
            cJSON_AddStringToObject(root, "dns", buf);
        }
    }

    // MAC STA
    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        cJSON_AddStringToObject(root, "mac", buf);
    }

    // SSID conectado
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
        cJSON_AddStringToObject(root, "ssid", (char*)ap.ssid);
        cJSON_AddNumberToObject(root, "rssi", ap.rssi);
    }

    // IP del AP
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif) {
        esp_netif_ip_info_t ap_ip;
        if (esp_netif_get_ip_info(ap_netif, &ap_ip) == ESP_OK) {
            sprintf(buf, IPSTR, IP2STR(&ap_ip.ip));
            cJSON_AddStringToObject(root, "ap_ip", buf);
        }
    }

    return root;
}
