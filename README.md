# SmartButton

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.2-blue)](https://github.com/espressif/esp-idf)
[![MCU](https://img.shields.io/badge/MCU-ESP32--C6-green)](https://www.espressif.com/en/products/socs/esp32-c6)
[![License](https://img.shields.io/badge/license-MIT-orange)](LICENSE)

Botón IoT configurable basado en ESP32-C6. Pulsa un botón físico y ejecuta una petición HTTP a la URL que quieras. Configurable desde el móvil vía portal web captivo.

## Características

- **2 botones físicos** con URL configurable cada uno (GET o POST)
- **Portal web captivo** — al conectarte al AP se abre automáticamente
- **Escaneo WiFi** desde la web para seleccionar tu red
- **Autenticación** con usuario/contraseña (por defecto `admin`/`admin`)
- **Botón de test** para probar las URLs desde la web antes de guardar
- **Factory reset** manteniendo ambos botones 8 segundos
- **Fallback automático** — si no conecta al WiFi, vuelve a modo AP
- **OTA dual** preparado para actualizaciones over-the-air

## Hardware

### MCU

- **ESP32-C6** (RISC-V, WiFi 6, BLE 5)
- Flash: **16MB**

### Pinout

| Función | GPIO | Configuración |
|---------|------|---------------|
| Botón 1 | **GPIO 4** | Pull-up interno, active-low |
| Botón 2 | **GPIO 5** | Pull-up interno, active-low |
| LED | **GPIO 8** | Salida digital |

### Esquema de conexión

```
ESP32-C6
┌──────────┐
│ GPIO 4 ├──── BTN1 ──── GND
│ GPIO 5 ├──── BTN2 ──── GND
│ GPIO 8 ├──── LED(+) ── R(330Ω) ── GND
│ │
│ 3V3 ├──── (pull-up interno habilitado)
│ GND ├──── GND común
└──────────┘
```

Los botones son **normalmente abiertos** (NO). Al pulsar conectan el GPIO a GND. No necesitan resistencia externa — el firmware activa el pull-up interno.

## Instalación

### Requisitos

- [ESP-IDF v5.2+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/)
- ESP32-C6 con flash de 16MB

### Clonar y compilar

```bash
git clone https://github.com/soyunomas/smartbutton.git
cd smartbutton

# Configurar entorno ESP-IDF (si no lo tienes activo)
. $HOME/esp/esp-idf/export.sh

# Compilar
idf.py build

# Flashear (primera vez, borra NVS)
idf.py -p /dev/ttyUSB0 erase_flash flash monitor

# Flashear (actualizaciones, conserva configuración)
idf.py -p /dev/ttyUSB0 flash monitor
```

## Uso

### Primera configuración

1. **Alimenta** el ESP32-C6
2. Busca la red WiFi **`SmartButton-XXXX`** desde tu móvil
3. El portal de configuración se abre automáticamente (o ve a `192.168.4.1`)
4. Inicia sesión con **admin** / **admin**
5. **Busca redes WiFi**, selecciona la tuya e introduce la contraseña
6. Configura las **URLs de los botones** (GET/POST, payload, timeout)
7. Guarda y el dispositivo se reinicia conectado a tu WiFi

### Uso normal

- **Pulsación corta Botón 1** → ejecuta la petición HTTP configurada para el botón 1
- **Pulsación corta Botón 2** → ejecuta la petición HTTP configurada para el botón 2
- **LED fijo** → conectado al WiFi, listo
- **LED parpadeo rápido** → conectando al WiFi

### Factory Reset

Mantén **ambos botones pulsados simultáneamente**:

| Tiempo | LED | Acción |
|--------|-----|--------|
| 0 – 5s | Parpadeo lento (500ms) | Puedes soltar para cancelar |
| 5 – 8s | Parpadeo rápido (100ms) | Aviso: va a borrar todo |
| 8s+ | — | Borra NVS, reinicia, entra en modo AP |

El factory reset borra: configuración WiFi, URLs de botones y credenciales de admin.

## Configuración de botones

Cada botón permite configurar:

| Campo | Descripción |
|-------|-------------|
| **URL** | Dirección HTTP a llamar (ej: `http://192.168.1.50/api/accion`) |
| **Método** | `GET` o `POST` |
| **Payload** | Cuerpo de la petición para POST (JSON) |
| **Timeout** | Tiempo máximo de espera en segundos (1-30s, default: 5s) |

### Feedback LED tras petición

- **Parpadeo único largo** (1s) → petición exitosa (HTTP 2xx)
- **Triple parpadeo rápido** → error (timeout, conexión fallida o HTTP no-2xx)

## Arquitectura

```
components/
├── app_core/      # Máquina de estados global, event group
├── app_nvs/       # Persistencia NVS (WiFi, botones, admin)
├── app_wifi/      # Modos STA/APSTA, escaneo WiFi, fallback
├── app_web/       # Servidor HTTP, portal web, autenticación
├── app_http/      # Cliente HTTP para ejecutar acciones de botones
├── app_buttons/   # Polling GPIO, detección pulsación y reset
├── app_led/       # Feedback visual, blink patterns
└── app_dns/       # Servidor DNS captive portal
```

### Estados del sistema

```
INIT → NO_CONFIG → AP_MODE (portal de configuración)
     → CONNECTING → NORMAL → HTTP_REQ (ejecutando petición)
                           → RESET_WARNING → FACTORY_RESET
                  → AP_MODE (fallback tras 5 reintentos)
```

### API Web

| Endpoint | Método | Descripción |
|----------|--------|-------------|
| `/` | GET | Portal de configuración (HTML) |
| `/api/verify` | GET | Verificar credenciales |
| `/api/scan` | GET | Escanear redes WiFi |
| `/api/wifi` | POST | Guardar configuración WiFi |
| `/api/btn?id=N` | GET | Leer configuración del botón N |
| `/api/btn` | POST | Guardar configuración de botón |
| `/api/test` | POST | Probar petición HTTP |
| `/api/admin` | POST | Cambiar credenciales admin |

Todos los endpoints (excepto `/` y captive portal) requieren autenticación HTTP Basic Auth.

### Tabla de particiones

| Partición | Tipo | Tamaño |
|-----------|------|--------|
| nvs | NVS | 24 KB |
| factory | App | 2 MB |
| ota_0 | OTA | 2 MB |
| ota_1 | OTA | 2 MB |
| storage | SPIFFS | 1 MB |

## Licencia

MIT
