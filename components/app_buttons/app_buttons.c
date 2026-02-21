#include "app_buttons.h"
#include "driver/gpio.h"
#include "app_core.h"
#include "app_http.h"
#include "app_led.h"
#include "esp_log.h"

#define BTN1_GPIO 4
#define BTN2_GPIO 5
#define POLL_RATE_MS 50
#define DEBOUNCE_MS 200
#define RESET_TIME_MS 8000
#define WARN_TIME_MS 5000

static const char *TAG = "BUTTONS";

static void button_task(void *arg) {
    uint32_t both_duration = 0;
    int prev_b1 = 1, prev_b2 = 1;
    uint32_t b1_press_time = 0, b2_press_time = 0;
    bool both_held = false;

    while (1) {
        int b1 = gpio_get_level(BTN1_GPIO);
        int b2 = gpio_get_level(BTN2_GPIO);

        // Ambos presionados → lógica de reset
        if (b1 == 0 && b2 == 0) {
            both_held = true;
            both_duration += POLL_RATE_MS;

            if (both_duration > RESET_TIME_MS) {
                app_set_state(STATE_FACTORY_RESET);
                vTaskDelay(portMAX_DELAY);
            } else if (both_duration > WARN_TIME_MS) {
                // 5-8s: parpadeo rápido
                app_led_set_blink(100, 100);
            } else {
                // 0-5s: parpadeo lento
                app_led_set_blink(500, 500);
            }
        } else {
            // Si estábamos en hold de reset y soltamos antes de tiempo
            if (both_held) {
                both_held = false;
                both_duration = 0;
                if (app_get_state() == STATE_RESET_WARNING) {
                    app_set_state(STATE_NORMAL);
                }
                app_led_update_state(app_get_state());
            }

            // Pulsación corta botón 1 (flanco de subida: estaba presionado, ahora suelto)
            if (prev_b1 == 0 && b1 == 1 && b2 == 1) {
                if (b1_press_time >= DEBOUNCE_MS && app_get_state() == STATE_NORMAL) {
                    ESP_LOGI(TAG, "BTN1 short press");
                    app_http_trigger(1);
                }
                b1_press_time = 0;
            }

            // Pulsación corta botón 2 (flanco de subida)
            if (prev_b2 == 0 && b2 == 1 && b1 == 1) {
                if (b2_press_time >= DEBOUNCE_MS && app_get_state() == STATE_NORMAL) {
                    ESP_LOGI(TAG, "BTN2 short press");
                    app_http_trigger(2);
                }
                b2_press_time = 0;
            }

            // Contar tiempo de pulsación individual
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

    xTaskCreate(button_task, "btn_task", 3072, NULL, 5, NULL);
}
