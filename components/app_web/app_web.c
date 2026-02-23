#include "app_web.h"
#include "html_ui.h"
#include "esp_http_server.h"
#include "app_nvs.h"
#include "app_wifi.h"
#include "app_core.h"
#include "app_http.h"
#include "app_led.h"
#include "esp_log.h"
#include "cJSON.h"
#include "mbedtls/base64.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include <string.h>

static const char *TAG = "WEB";

static bool check_auth(httpd_req_t *req) {
    char auth_hdr[128] = {0};
    if (httpd_req_get_hdr_value_str(req, "Authorization", auth_hdr, sizeof(auth_hdr)) != ESP_OK) {
        return false;
    }
    if (strncmp(auth_hdr, "Basic ", 6) != 0) return false;

    unsigned char decoded[96];
    size_t decoded_len = 0;
    if (mbedtls_base64_decode(decoded, sizeof(decoded) - 1, &decoded_len,
                              (unsigned char *)auth_hdr + 6, strlen(auth_hdr + 6)) != 0) {
        return false;
    }
    decoded[decoded_len] = 0;

    char *sep = strchr((char *)decoded, ':');
    if (!sep) return false;
    *sep = 0;
    const char *rx_user = (char *)decoded;
    const char *rx_pass = sep + 1;

    admin_config_t admin;
    app_nvs_get_admin(&admin);

    return (strcmp(rx_user, admin.user) == 0 && strcmp(rx_pass, admin.pass) == 0);
}

static esp_err_t send_auth_required(httpd_req_t *req) {
    httpd_resp_set_status(req, "401 Unauthorized");
    httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"SmartButton\"");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "Login required", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

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
    char buf[1024];
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
    cJSON *jcooldown = cJSON_GetObjectItem(root, "cooldown");

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

        cfg.cooldown_ms = cJSON_IsNumber(jcooldown) ? jcooldown->valueint : 2000;
        if (cfg.cooldown_ms < 500) cfg.cooldown_ms = 500;
        if (cfg.cooldown_ms > 60000) cfg.cooldown_ms = 60000;

        app_nvs_save_button(jid->valueint, &cfg);
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
        cJSON_AddNumberToObject(root, "cooldown", cfg.cooldown_ms > 0 ? cfg.cooldown_ms : 2000);
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
    char buf[1024];
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

static esp_err_t led_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send(req, "{\"ok\":false}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    cJSON *joff = cJSON_GetObjectItem(root, "off");
    if (cJSON_IsTrue(joff)) {
        app_led_off();
    } else {
        cJSON *jr = cJSON_GetObjectItem(root, "r");
        cJSON *jg = cJSON_GetObjectItem(root, "g");
        cJSON *jb = cJSON_GetObjectItem(root, "b");
        uint8_t r = cJSON_IsNumber(jr) ? (uint8_t)jr->valueint : 0;
        uint8_t g = cJSON_IsNumber(jg) ? (uint8_t)jg->valueint : 0;
        uint8_t b = cJSON_IsNumber(jb) ? (uint8_t)jb->valueint : 0;
        app_led_set_color(r, g, b);
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t netinfo_get_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    cJSON *info = app_wifi_get_netinfo();
    char *json = cJSON_PrintUnformatted(info);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    cJSON_Delete(info);
    return ESP_OK;
}

static esp_err_t admin_get_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    admin_config_t admin;
    app_nvs_get_admin(&admin);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "user", admin.user);
    cJSON_AddNumberToObject(root, "reset_time", admin.reset_time_ms / 1000);
    cJSON_AddStringToObject(root, "ap_ssid", admin.ap_ssid);
    cJSON_AddStringToObject(root, "ap_pass", admin.ap_pass);
    cJSON_AddBoolToObject(root, "pure_client", admin.pure_client);
    cJSON_AddBoolToObject(root, "deep_sleep", admin.deep_sleep);
    
    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t admin_post_handler(httpd_req_t *req) {
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

    cJSON *juser = cJSON_GetObjectItem(root, "user");
    cJSON *jpass = cJSON_GetObjectItem(root, "pass");
    cJSON *jreset = cJSON_GetObjectItem(root, "reset_time");
    cJSON *jap_ssid = cJSON_GetObjectItem(root, "ap_ssid");
    cJSON *jap_pass = cJSON_GetObjectItem(root, "ap_pass");
    cJSON *jpure = cJSON_GetObjectItem(root, "pure_client");
    cJSON *jdeep = cJSON_GetObjectItem(root, "deep_sleep");

    admin_config_t current;
    app_nvs_get_admin(&current);

    const char *user = (cJSON_IsString(juser) && juser->valuestring[0]) ? juser->valuestring : current.user;
    const char *pass = (cJSON_IsString(jpass) && jpass->valuestring[0]) ? jpass->valuestring : current.pass;
    int reset_ms = cJSON_IsNumber(jreset) ? jreset->valueint * 1000 : current.reset_time_ms;
    
    const char *ap_ssid = cJSON_IsString(jap_ssid) ? jap_ssid->valuestring : current.ap_ssid;
    const char *ap_pass = cJSON_IsString(jap_pass) ? jap_pass->valuestring : current.ap_pass;
    bool pure_client = cJSON_IsBool(jpure) ? cJSON_IsTrue(jpure) : current.pure_client;
    bool deep_sleep = cJSON_IsBool(jdeep) ? cJSON_IsTrue(jdeep) : current.deep_sleep;

    if (reset_ms < 3000) reset_ms = 3000;
    if (reset_ms > 60000) reset_ms = 60000;

    app_nvs_save_admin(user, pass, reset_ms, ap_ssid, ap_pass, pure_client, deep_sleep);
    ESP_LOGI(TAG, "Admin credentials & AP settings updated");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(root);

    vTaskDelay(pdMS_TO_TICKS(1500));
    esp_restart();
    return ESP_OK;
}

static esp_err_t factory_reset_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);
    ESP_LOGW(TAG, "Factory reset requested from web");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
    app_nvs_clear_all();
    esp_restart();
    return ESP_OK;
}

static esp_err_t ota_post_handler(httpd_req_t *req) {
    if (!check_auth(req)) return send_auth_required(req);

    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    char buf[1024];
    int remaining = req->content_len;

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "OTA partition not found");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Writing OTA to partition subtype %d at offset 0x%lx",
             update_partition->subtype, update_partition->address);

    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (remaining > 0) {
        int recv_len = httpd_req_recv(req, buf, (remaining > sizeof(buf)) ? sizeof(buf) : remaining);
        if (recv_len <= 0) {
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) continue;
            esp_ota_end(update_handle);
            return ESP_FAIL;
        }

        err = esp_ota_write(update_handle, buf, recv_len);
        if (err != ESP_OK) {
            esp_ota_end(update_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        remaining -= recv_len;
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed (%s)", esp_err_to_name(err));
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)", esp_err_to_name(err));
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "OTA Success. Rebooting...");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

void app_web_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 24;
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

        httpd_uri_t uri_led = { .uri = "/api/led", .method = HTTP_POST, .handler = led_post_handler };
        httpd_register_uri_handler(server, &uri_led);

        httpd_uri_t uri_netinfo = { .uri = "/api/netinfo", .method = HTTP_GET, .handler = netinfo_get_handler };
        httpd_register_uri_handler(server, &uri_netinfo);

        httpd_uri_t uri_admin_get = { .uri = "/api/admin", .method = HTTP_GET, .handler = admin_get_handler };
        httpd_register_uri_handler(server, &uri_admin_get);

        httpd_uri_t uri_admin_post = { .uri = "/api/admin", .method = HTTP_POST, .handler = admin_post_handler };
        httpd_register_uri_handler(server, &uri_admin_post);

        httpd_uri_t uri_freset = { .uri = "/api/factory_reset", .method = HTTP_POST, .handler = factory_reset_post_handler };
        httpd_register_uri_handler(server, &uri_freset);

        httpd_uri_t uri_ota = { .uri = "/api/ota", .method = HTTP_POST, .handler = ota_post_handler };
        httpd_register_uri_handler(server, &uri_ota);

        const char *captive_uris[] = {
            "/generate_204", "/gen_204", "/hotspot-detect.html",
            "/connecttest.txt", "/redirect", "/canonical.html"
        };
        for (int i = 0; i < sizeof(captive_uris) / sizeof(captive_uris[0]); i++) {
            httpd_uri_t uri = { .uri = captive_uris[i], .method = HTTP_GET, .handler = captive_redirect_handler };
            httpd_register_uri_handler(server, &uri);
        }

        ESP_LOGI(TAG, "Web Server Started");
    }
}
