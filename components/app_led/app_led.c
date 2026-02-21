#include "app_led.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO 8 // LED RGB Integrado en ESP32-C6 DevKit suele ser GPIO 8
// Nota: Si es LED RGB WS2812 (Neopixel) se requiere driver RMT.
// Asumiremos LED simple para este ejemplo base.

static int blink_on_ms = 0;
static int blink_off_ms = 0;
static bool one_shot = false;

static void led_task(void *arg) {
    while (1) {
        if (blink_on_ms > 0) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(blink_on_ms));
            if (blink_off_ms > 0) {
                gpio_set_level(LED_GPIO, 0);
                vTaskDelay(pdMS_TO_TICKS(blink_off_ms));
            }
        } else {
            gpio_set_level(LED_GPIO, 0); // Apagado por defecto
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void app_led_init(void) {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}

void app_led_update_state(system_state_t state) {
    switch (state) {
        case STATE_AP_MODE:       blink_on_ms = 1000; blink_off_ms = 1000; break; // Lento
        case STATE_CONNECTING:    blink_on_ms = 200;  blink_off_ms = 200;  break; // Rápido
        case STATE_NORMAL:        blink_on_ms = 0;    blink_off_ms = 0;    gpio_set_level(LED_GPIO, 1); break; // Fijo ON
        case STATE_RESET_WARNING: blink_on_ms = 100;  blink_off_ms = 100;  break; // Muy rápido
        default: break;
    }
}

void app_led_signal_success(void) {
    // Parpadeo único lento
    gpio_set_level(LED_GPIO, 0); vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(LED_GPIO, 1); vTaskDelay(pdMS_TO_TICKS(1000));
}

void app_led_signal_error(void) {
    // Parpadeo triple rápido
    for(int i=0; i<3; i++) {
        gpio_set_level(LED_GPIO, 1); vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_GPIO, 0); vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_led_set_blink(int on_ms, int off_ms) {
    blink_on_ms = on_ms;
    blink_off_ms = off_ms;
}
