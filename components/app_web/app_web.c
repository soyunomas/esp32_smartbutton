#include "app_web.h"
#include "html_ui.h"
#include "esp_http_server.h"
#include "app_nvs.h"
#include "app_wifi.h"
#include "app_core.h"
#include "app_http.h"
#include "esp_log.h"
#include "cJSON.h"
#include "mbedtls/base64.h"
#include <string.h>

static const char *TAG = "WEB";

// Verifica HTTP Basic Auth. Devuelve true si OK.
static bool check_auth(httpd_req_t *req) {
    char auth_hdr[128] = {0};
    if (httpd_req_get_hdr_value_str(req, "Authorization", auth_hdr, sizeof(auth_hdr)) != ESP_OK) {
        return false;
    }

    // Formato: "Basic base64(user:pass)"
    if (strncmp(auth_hdr, "Basic ", 6) != 0) return false;

    unsigned char decoded[96];
    size_t decoded_len = 0;
    if (mbedtls_base64_decode(decoded, sizeof(decoded) - 1, &decoded_len,
                              (unsigned char *)auth_hdr + 6, strlen(auth_hdr + 6)) != 0) {
        return false;
    }
    decoded[decoded_len] = 0;

    // Separar user:pass
    char *sep = strchr((char *)decoded, ':');
    if (!sep) return false;
    *sep = 0;
    const char *rx_user = (char *)decoded;
    const char *rx_pass = sep + 1;

    admin_config_t admin;
    app_nvs_get_admin(&admin);

    return (strcmp(rx_user, admin.user) == 0 && strcmp(rx_pass, admin.pass) == 0);
}

// Envía 401 pidiendo autenticación
static esp_err_t send_auth_required(httpd_req_t *req) {
    httpd_resp_set_status(req, "401 Unauthorized");
    httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"SmartButton\"");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "Login required", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Redirige al portal de configuración (captive portal)
static esp_err_t captive_redirect_handler(httpd_req_t *req) {
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t verify_get_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t scan_get_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    cJSON *arr = app_wifi_scan();
    char *json = cJSON_PrintUnformatted(arr);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    cJSON_Delete(arr);
    return ESP_OK;
}

static esp_err_t wifi_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
    cJSON *pass = cJSON_GetObjectItem(root, "pass");

    if (cJSON_IsString(ssid) && ssid->valuestring[0]) {
        const char *p = (cJSON_IsString(pass)) ? pass->valuestring : "";
        app_nvs_save_wifi(ssid->valuestring, p);
        ESP_LOGI(TAG, "WiFi config saved: %s", ssid->valuestring);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
        cJSON_Delete(root);
        vTaskDelay(pdMS_TO_TICKS(1500));
        esp_restart();
        return ESP_OK;
    }

    cJSON_Delete(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t btn_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    cJSON *jid = cJSON_GetObjectItem(root, "id");
    cJSON *jurl = cJSON_GetObjectItem(root, "url");
    cJSON *jmethod = cJSON_GetObjectItem(root, "method");
    cJSON *jpayload = cJSON_GetObjectItem(root, "payload");
    cJSON *jtimeout = cJSON_GetObjectItem(root, "timeout");

    if (cJSON_IsNumber(jid) && cJSON_IsString(jurl)) {
        button_config_t cfg = {0};
        strlcpy(cfg.url, jurl->valuestring, sizeof(cfg.url));
        cfg.method = cJSON_IsNumber(jmethod) ? jmethod->valueint : 0;
        if (cJSON_IsString(jpayload)) {
            strlcpy(cfg.payload, jpayload->valuestring, sizeof(cfg.payload));
        }
        cfg.timeout_ms = cJSON_IsNumber(jtimeout) ? jtimeout->valueint : 5000;
        if (cfg.timeout_ms < 1000) cfg.timeout_ms = 1000;
        if (cfg.timeout_ms > 30000) cfg.timeout_ms = 30000;
        app_nvs_save_button(jid->valueint, &cfg);
        ESP_LOGI(TAG, "Button %d saved (timeout=%dms)", jid->valueint, cfg.timeout_ms);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
    }

    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t btn_get_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    char param[8] = {0};
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) != ESP_OK) {
        httpd_resp_send(req, "{}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    char val[4] = {0};
    httpd_query_key_value(param, "id", val, sizeof(val));
    int btn_id = atoi(val);

    button_config_t cfg = {0};
    cJSON *root = cJSON_CreateObject();

    if (app_nvs_get_button_config(btn_id, &cfg) == ESP_OK) {
        cJSON_AddStringToObject(root, "url", cfg.url);
        cJSON_AddNumberToObject(root, "method", cfg.method);
        cJSON_AddStringToObject(root, "payload", cfg.payload);
        cJSON_AddNumberToObject(root, "timeout", cfg.timeout_ms > 0 ? cfg.timeout_ms : 5000);
    }

    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t test_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send(req, "{\"ok\":false,\"status\":-1}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    cJSON *jurl = cJSON_GetObjectItem(root, "url");
    cJSON *jmethod = cJSON_GetObjectItem(root, "method");
    cJSON *jpayload = cJSON_GetObjectItem(root, "payload");
    cJSON *jtimeout = cJSON_GetObjectItem(root, "timeout");

    button_config_t cfg = {0};
    if (cJSON_IsString(jurl)) {
        strlcpy(cfg.url, jurl->valuestring, sizeof(cfg.url));
    }
    cfg.method = cJSON_IsNumber(jmethod) ? jmethod->valueint : 0;
    if (cJSON_IsString(jpayload)) {
        strlcpy(cfg.payload, jpayload->valuestring, sizeof(cfg.payload));
    }
    cfg.timeout_ms = cJSON_IsNumber(jtimeout) ? jtimeout->valueint : 5000;

    int status = app_http_test_sync(&cfg);

    cJSON *resp = cJSON_CreateObject();
    cJSON_AddBoolToObject(resp, "ok", status >= 200 && status < 300);
    cJSON_AddNumberToObject(resp, "status", status);
    char *json = cJSON_PrintUnformatted(resp);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    cJSON_Delete(resp);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t admin_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    cJSON *juser = cJSON_GetObjectItem(root, "user");
    cJSON *jpass = cJSON_GetObjectItem(root, "pass");

    if (cJSON_IsString(juser) && juser->valuestring[0] &&
        cJSON_IsString(jpass) && jpass->valuestring[0]) {
        app_nvs_save_admin(juser->valuestring, jpass->valuestring);
        ESP_LOGI(TAG, "Admin credentials updated: %s", juser->valuestring);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
    }

    cJSON_Delete(root);
    return ESP_OK;
}

void app_web_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.stack_size = 8192;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
        httpd_register_uri_handler(server, &uri_root);

        httpd_uri_t uri_verify = { .uri = "/api/verify", .method = HTTP_GET, .handler = verify_get_handler };
        httpd_register_uri_handler(server, &uri_verify);

        httpd_uri_t uri_scan = { .uri = "/api/scan", .method = HTTP_GET, .handler = scan_get_handler };
        httpd_register_uri_handler(server, &uri_scan);

        httpd_uri_t uri_wifi = { .uri = "/api/wifi", .method = HTTP_POST, .handler = wifi_post_handler };
        httpd_register_uri_handler(server, &uri_wifi);

        httpd_uri_t uri_btn_post = { .uri = "/api/btn", .method = HTTP_POST, .handler = btn_post_handler };
        httpd_register_uri_handler(server, &uri_btn_post);

        httpd_uri_t uri_btn_get = { .uri = "/api/btn", .method = HTTP_GET, .handler = btn_get_handler };
        httpd_register_uri_handler(server, &uri_btn_get);

        httpd_uri_t uri_test = { .uri = "/api/test", .method = HTTP_POST, .handler = test_post_handler };
        httpd_register_uri_handler(server, &uri_test);

        httpd_uri_t uri_admin = { .uri = "/api/admin", .method = HTTP_POST, .handler = admin_post_handler };
        httpd_register_uri_handler(server, &uri_admin);

        // Captive portal: URLs de detección de conectividad
        const char *captive_uris[] = {
            "/generate_204",        // Android
            "/gen_204",             // Android
            "/hotspot-detect.html", // iOS/macOS
            "/connecttest.txt",     // Windows
            "/redirect",            // Windows
            "/canonical.html",      // Firefox
        };
        for (int i = 0; i < sizeof(captive_uris) / sizeof(captive_uris[0]); i++) {
            httpd_uri_t uri = { .uri = captive_uris[i], .method = HTTP_GET, .handler = captive_redirect_handler };
            httpd_register_uri_handler(server, &uri);
        }

        ESP_LOGI(TAG, "Web Server Started");
    }
}
