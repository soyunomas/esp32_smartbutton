#include "app_core.h"
#include "app_nvs.h"
#include "app_wifi.h"
#include "app_web.h"
#include "app_buttons.h"
#include "app_led.h"
#include "app_dns.h"
#include "app_http.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h" // Importante para leer el motivo de reinicio hardware
#include "driver/gpio.h"

void app_main(void) {
    // Detección temprana del botón de wakeup: leemos GPIO ANTES de cualquier
    // init para capturar el nivel mientras el usuario aún pulsa el botón
    bool is_wakeup = (esp_reset_reason() == ESP_RST_DEEPSLEEP);
    int wakeup_btn = 0;

    if (is_wakeup) {
        int b1 = gpio_get_level(GPIO_NUM_4);
        int b2 = gpio_get_level(GPIO_NUM_5);
        uint64_t wakeup_pin_mask = esp_sleep_get_gpio_wakeup_status();

        if (wakeup_pin_mask & (1ULL << 5)) {
            wakeup_btn = 2;
        } else if (wakeup_pin_mask & (1ULL << 4)) {
            wakeup_btn = 1;
        } else if (b2 == 0) {
            wakeup_btn = 2;
        } else if (b1 == 0) {
            wakeup_btn = 1;
        } else {
            wakeup_btn = 1;
        }

        ESP_LOGI("MAIN", "Woke up from deep sleep. BTN: %d (mask: %llu, GPIO4: %d, GPIO5: %d)",
                 wakeup_btn, (unsigned long long)wakeup_pin_mask, b1, b2);
    }

    // Inicialización de sistema
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    app_nvs_init();
    app_event_group = xEventGroupCreate();
    
    app_set_state_callback(app_led_update_state);
    
    // Liberamos la retención individual de los GPIO
    gpio_hold_dis(GPIO_NUM_4);
    gpio_hold_dis(GPIO_NUM_5);

    app_led_init();
    app_buttons_init();

    nvs_wifi_config_t conf;
    bool configured = app_nvs_get_wifi_config(&conf);

    admin_config_t admin;
    app_nvs_get_admin(&admin);

    if (admin.deep_sleep && !is_wakeup) {
        ESP_LOGI("MAIN", "Normal boot. Will stay awake for 3 mins before sleeping.");
    }

    if (!configured) {
        app_set_state(STATE_NO_CONFIG);
        app_wifi_start_ap();
        app_dns_start();
    } else {
        app_set_state(STATE_CONNECTING);
        app_wifi_start_sta();
    }
    app_web_start();

    uint32_t awake_time = 0;

    // Loop principal (Watchdog lógico)
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(app_event_group, EVENT_HTTP_DONE, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
        awake_time++;

        if (app_get_state() == STATE_FACTORY_RESET) {
            ESP_LOGW("MAIN", "ERASING NVS...");
            app_nvs_clear_all();
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        }

        if (admin.deep_sleep && configured) {
            
            // Disparo automático de la petición si nos despertó el usuario
            if (wakeup_btn != 0 && app_get_state() == STATE_NORMAL) {
                ESP_LOGI("MAIN", "Triggering HTTP for wakeup BTN%d", wakeup_btn);
                app_buttons_simulate_press(wakeup_btn);
                wakeup_btn = 0; 
            }

            bool should_sleep = false;
            
            // La misma variable del hardware se usa para decidir si duerme rápido
            if (is_wakeup) {
                // Modo rápido: duerme al instante tras la petición web
                if ((bits & EVENT_HTTP_DONE) || awake_time > 30) {
                    should_sleep = true;
                }
            } else {
                // Modo Configuración (encendido frío): Permite entrar a la web por 3 mins
                if ((bits & EVENT_HTTP_DONE) || awake_time > 180) {
                    should_sleep = true;
                }
            }

            if (should_sleep) {
                ESP_LOGI("MAIN", "Entering Deep Sleep...");
                vTaskDelay(pdMS_TO_TICKS(1500)); 
                
                app_led_set_color(0, 0, 30); 
                vTaskDelay(pdMS_TO_TICKS(200));
                app_led_off();               
                vTaskDelay(pdMS_TO_TICKS(100)); 

                const uint64_t gpio_mask = (1ULL << 4) | (1ULL << 5);
                esp_deep_sleep_enable_gpio_wakeup(gpio_mask, ESP_GPIO_WAKEUP_GPIO_LOW);
                
                // Retención segura de pullups
                gpio_hold_en(GPIO_NUM_4);
                gpio_hold_en(GPIO_NUM_5);

                esp_deep_sleep_start();
            }
        }
    }
}
