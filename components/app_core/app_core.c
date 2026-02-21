#include "app_core.h"
#include "esp_log.h"

static const char *TAG = "CORE";
system_state_t current_state = STATE_INIT;
EventGroupHandle_t app_event_group;

void app_set_state(system_state_t new_state) {
    if (current_state != new_state) {
        current_state = new_state;
        ESP_LOGI(TAG, "State Transition -> %d", new_state);
        // Aquí podríamos notificar al LED inmediatamente si fuera necesario
        // app_led_update_state(new_state); // Se haría vía callback o polling
    }
}

system_state_t app_get_state(void) {
    return current_state;
}
