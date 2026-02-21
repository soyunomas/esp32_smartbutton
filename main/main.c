#include "app_core.h"
#include "app_nvs.h"
#include "app_wifi.h"
#include "app_web.h"
#include "app_buttons.h"
#include "app_led.h"
#include "app_dns.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"

void app_main(void) {
    // Inicialización de sistema
    ESP_ERROR_CHECK(esp_netif_init());
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
        app_dns_start();
    } else {
        app_set_state(STATE_CONNECTING);
        app_wifi_start_sta();
    }
    app_web_start();

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
