#include "app_buttons.h"
#include "driver/gpio.h"
#include "app_core.h"
#include "app_http.h"
#include "app_led.h"
#include "app_nvs.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BTN1_GPIO 4
#define BTN2_GPIO 5
#define POLL_RATE_MS 50
#define DEBOUNCE_MS 200

static const char *TAG = "BUTTONS";
static int64_t last_trigger_time_1 = 0;
static int64_t last_trigger_time_2 = 0;

static bool can_trigger(int btn_id) {
    if (app_get_state() == STATE_HTTP_REQ) {
        ESP_LOGW(TAG, "Ignored: System Busy");
        app_led_set_blink(50, 50); 
        vTaskDelay(pdMS_TO_TICKS(200)); 
        app_led_update_state(app_get_state());
        return false;
    }

    button_config_t cfg;
    if (app_nvs_get_button_config(btn_id, &cfg) != ESP_OK) {
        cfg.cooldown_ms = 2000; 
    }

    int64_t now = esp_timer_get_time() / 1000;
    int64_t *last_time = (btn_id == 1) ? &last_trigger_time_1 : &last_trigger_time_2;

    if ((now - *last_time) < cfg.cooldown_ms) {
        ESP_LOGW(TAG, "Ignored: Cooldown active");
        return false;
    }

    *last_time = now;
    return true;
}

static void button_task(void *arg) {
    uint32_t both_duration = 0;
    int prev_b1 = 1, prev_b2 = 1;
    uint32_t b1_press_time = 0, b2_press_time = 0;
    bool both_held = false;
    uint32_t reset_time_ms = 8000;
    uint32_t warn_time_ms = 5000;

    while (1) {
        int b1 = gpio_get_level(BTN1_GPIO);
        int b2 = gpio_get_level(BTN2_GPIO);

        if (b1 == 0 && b2 == 0) {
            if (!both_held) {
                both_held = true;
                both_duration = 0;
                
                // Leemos el tiempo configurable solo la primera vez que se detectan pulsados
                admin_config_t admin;
                app_nvs_get_admin(&admin);
                reset_time_ms = admin.reset_time_ms > 0 ? admin.reset_time_ms : 8000;
                
                // Empezamos a avisar de forma violenta (Rojo rápido) 3 segundos antes del borrado.
                warn_time_ms = reset_time_ms > 3000 ? reset_time_ms - 3000 : reset_time_ms / 2;
                
                app_led_set_blink(500, 500); // Start slow blink mode override
            }
            both_duration += POLL_RATE_MS;

            if (both_duration > reset_time_ms) {
                app_set_state(STATE_FACTORY_RESET);
                vTaskDelay(portMAX_DELAY);
            } else if (both_duration > warn_time_ms && app_get_state() != STATE_RESET_WARNING) {
                app_set_state(STATE_RESET_WARNING);
            }
        } else {
            // Se soltaron los botones
            if (both_held) {
                both_held = false;
                both_duration = 0;
                // Si estaba mostrando warning, lo cancelamos y devolvemos al estado habitual
                if (app_get_state() == STATE_RESET_WARNING) {
                    app_set_state(STATE_NORMAL);
                } else {
                    app_led_update_state(app_get_state());
                }
            }

            // Pulsación simple de botón 1
            if (prev_b1 == 0 && b1 == 1 && b2 == 1) {
                if (b1_press_time >= DEBOUNCE_MS && app_get_state() == STATE_NORMAL) {
                    if (can_trigger(1)) {
                        ESP_LOGI(TAG, "BTN1 Triggered");
                        app_http_trigger(1);
                    }
                }
                b1_press_time = 0;
            }

            // Pulsación simple de botón 2
            if (prev_b2 == 0 && b2 == 1 && b1 == 1) {
                if (b2_press_time >= DEBOUNCE_MS && app_get_state() == STATE_NORMAL) {
                    if (can_trigger(2)) {
                        ESP_LOGI(TAG, "BTN2 Triggered");
                        app_http_trigger(2);
                    }
                }
                b2_press_time = 0;
            }

            if (b1 == 0 && b2 == 1) b1_press_time += POLL_RATE_MS;
            if (b2 == 0 && b1 == 1) b2_press_time += POLL_RATE_MS;
        }

        prev_b1 = b1;
        prev_b2 = b2;
        vTaskDelay(pdMS_TO_TICKS(POLL_RATE_MS));
    }
}

void app_buttons_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTN1_GPIO) | (1ULL << BTN2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    xTaskCreate(button_task, "btn_task", 4096, NULL, 5, NULL);
}
