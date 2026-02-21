# AGENTS.md - SmartButton C6

## Descripción
Firmware ESP-IDF para ESP32-C6. Botón IoT configurable que ejecuta peticiones HTTP al pulsar botones físicos. Incluye portal web captivo para configuración WiFi y de botones.

## Target & Framework
- **MCU**: ESP32-C6 (RISC-V)
- **Framework**: ESP-IDF (CMake, FreeRTOS)
- **Flash**: 16MB, particiones custom con OTA dual + SPIFFS

## Arquitectura de Componentes

| Componente     | Responsabilidad                                      |
|----------------|------------------------------------------------------|
| `app_core`     | Máquina de estados global (`system_state_t`), event group |
| `app_nvs`      | Persistencia NVS (WiFi config, button configs)       |
| `app_wifi`     | Modos STA y AP (SoftAP abierto `SmartButton-XXXX`)  |
| `app_web`      | Servidor HTTP (portal config), UI en `html_ui.h`     |
| `app_http`     | Cliente HTTP para ejecutar acciones de botones       |
| `app_buttons`  | Polling GPIO de 2 botones (GPIO 4, 5), detección reset |
| `app_led`      | Feedback visual vía LED GPIO 8, blink patterns       |

## Convenciones

### Estructura de componentes
Cada componente sigue el patrón:
```
components/app_xxx/
├── CMakeLists.txt
├── app_xxx.c
└── include/
    └── app_xxx.h
```

### Código
- Lenguaje: **C** (no C++)
- Prefijo de funciones: `app_<componente>_<acción>()` (ej: `app_nvs_save_wifi()`)
- Logging: usar `ESP_LOGx(TAG, ...)` con `TAG` estático por archivo
- Tareas FreeRTOS: nombre descriptivo, stack mínimo viable
- Structs propios usar prefijo `nvs_` o `button_` para evitar colisiones con ESP-IDF
- Comentarios en español

### Estados del sistema
```
STATE_INIT → STATE_NO_CONFIG → STATE_AP_MODE
           → STATE_CONNECTING → STATE_NORMAL → STATE_HTTP_REQ
                                             → STATE_RESET_WARNING → STATE_FACTORY_RESET
                                             → STATE_ERROR
```

### GPIOs
- BTN1: GPIO 4 (pull-up, active-low)
- BTN2: GPIO 5 (pull-up, active-low)
- LED:  GPIO 8

## Comandos de Build
```bash
# Dentro del contenedor ESP-IDF o con IDF_PATH configurado
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
idf.py fullclean   # Limpiar build completo
```

## API Web
- `GET /` — Portal de configuración (HTML embebido)
- `POST /api/wifi` — Guardar configuración WiFi

## Notas Importantes
- `esp_crt_bundle` está deshabilitado en `app_http` (solo HTTP, no HTTPS por ahora)
- El script `repair_code.sh` regenera archivos fuente con fixes conocidos — no ejecutar salvo necesidad
- La UI HTML está embebida como string en `components/app_web/html_ui.h`
- Las dependencias de cada componente se declaran en su `CMakeLists.txt` con `REQUIRES`/`PRIV_REQUIRES`
- La tabla de particiones incluye OTA dual (ota_0, ota_1) de 2MB cada una
