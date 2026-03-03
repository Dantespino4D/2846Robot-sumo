# 2846 Robot Sumo

Firmware para robot de sumo autónomo basado en **ESP32 / ESP32-S3** con ESP-IDF y PlatformIO.

---

## Descripción del Proyecto

Este proyecto implementa el firmware completo de un robot de sumo competitivo. El robot utiliza múltiples sensores para detectar al oponente y los límites del ring, y toma decisiones en tiempo real mediante una máquina de estados ejecutada sobre FreeRTOS.

### Características principales

| Característica | Detalle |
|---|---|
| Microcontrolador | ESP32 / ESP32-S3 (seleccionable por entorno) |
| Framework | ESP-IDF 5.x + FreeRTOS |
| Herramienta de build | PlatformIO |
| Sensores de rival | 2× Ultrasónico HC-SR04 **o** 6× ToF VL53L (vía multiplexor I2C TCA9548A) |
| Sensores de límite | 2× Sensor de color TCS34725 (vía multiplexor I2C TCA9548A) |
| Control de motores | PWM (LEDC de ESP-IDF), con rampa de velocidad |
| Conectividad | WiFi + MQTT + mDNS + SmartConfig + OTA |
| Telemetría | Streaming de estado en tiempo real al broker MQTT |
| Configuración | NVS (Non-Volatile Storage) para parámetros ajustables en caliente |
| Música | Melodía de inicio mediante buzzer |

---

## Arquitectura del Software

El firmware corre múltiples tareas FreeRTOS, cada una fijada a un núcleo del ESP32:

```
Core 0                         Core 1
──────────────────────         ──────────────────────
SensorUltra (prio 2)           Robot / Lógica (prio 2)
Musica (prio 1)                Motores (prio 5)
Telemetria (prio 1)            SensorColor (prio 3)
```

### Módulos principales

- **`MaquinaEstados`** — Máquina de estados que decide el movimiento del robot según las lecturas de los sensores. Soporta tres estrategias: prototipo (EP), estrategia 1 (E1) y estrategia 2 (E2), seleccionables desde NVS sin recompilar.
- **`ControlMotores`** — Abstracción del control PWM de los dos motores DC. Define comandos de alto nivel: `DIR_A/B`, `ATAQUE_*`, `PRO_*`, `MAX_*`, `GIRO`.
- **`SensorLimite`** — Lee dos sensores de color para detectar el borde blanco del tatami.
- **`SensorRival`** — Interfaz abstracta para los sensores de detección de rival. Implementada por `SensorUltra` y `SensorTof`.
- **`Wifi` / `Mqtt`** — Conectividad para el modo de prueba/telemetría.
- **`Nvs`** — Capa de abstracción sobre NVS de ESP-IDF para guardar y leer parámetros de calibración y configuración.
- **`Telemetria`** — Publica el estado completo del robot (sensores, motores, WiFi, heap, temperatura) al broker MQTT cada 100 ms.

---

## Configuración de Credenciales

Las credenciales de WiFi y MQTT se configuran mediante **Kconfig** (sin escribirlas en el código fuente):

```bash
idf.py menuconfig
# → Robot Sumo Configuration → WiFi / MQTT
```

O editando directamente `sdkconfig.defaults` con tus propios valores antes de compilar. **No subas credenciales reales al repositorio.**

---

## Compilación y Flash

### Requisitos

- [PlatformIO](https://platformio.org/) instalado (extensión de VS Code o CLI)
- Drivers USB-UART instalados

### Compilar y flashear

```bash
# Para ESP32 estándar
pio run -e esp32 --target upload

# Para ESP32-S3
pio run -e esp32-s3 --target upload

# Monitor serial
pio device monitor
```

---

## Modos de Operación

El robot lee el modo de operación desde NVS al arrancar:

| Modo | Descripción | Activación |
|---|---|---|
| `0` | **Prueba / Telemetría** — Conecta WiFi + MQTT, transmite telemetría en tiempo real | Mantener botón pulsado al encender |
| `1` | **Combate** — Opera de forma autónoma sin WiFi | Por defecto |

En modo combate, presionar el botón de inicio calibra los sensores de color, espera 5 s y lanza el robot.

---

## Parámetros Ajustables (NVS)

Los siguientes parámetros se pueden modificar vía MQTT (topic de configuración) sin recompilar:

| Clave NVS | Descripción | Valor por defecto |
|---|---|---|
| `tiempos/ataque_ciego` | Tiempo de ataque sin detección (ms) | 2000 |
| `tiempos/retroceso` | Tiempo de retroceso al detectar borde (ms) | 400 |
| `tiempos/recta_star` | Duración del avance en patrón estrella (ms) | 2000 |
| `tiempos/giro_star` | Duración del giro en patrón estrella (ms) | 500 |
| `tiempos/estrategia` | Estrategia activa (0 = EP, 1 = E1, 2 = E2) | 0 |
| `sistema/modo` | Modo de operación (0 = prueba, 1 = combate) | 1 |
| `sistema/monitor` | Nivel de log (0=off, 1=error, 2=info, 3=verbose) | 2 |

---

## Opinión Técnica del Proyecto

### Fortalezas ✅

- **Arquitectura multitarea bien estructurada**: El uso de FreeRTOS con tareas específicas para cada subsistema (motores, sensores, lógica, telemetría) es correcto para un sistema embebido de tiempo real. Las prioridades están bien asignadas.
- **Abstracción del sensor rival**: La clase abstracta `SensorRival` permite intercambiar los sensores ultrasónicos por sensores ToF sin cambiar la lógica de combate — buen diseño orientado a objetos.
- **Configuración en caliente via NVS**: Poder ajustar tiempos y estrategia sin recompilar es muy útil durante las competencias.
- **Múltiples estrategias de combate**: Tener tres estrategias seleccionables (EP, E1, E2) permite adaptarse al oponente.
- **Telemetría en tiempo real**: El sistema MQTT para telemetría facilita enormemente el diagnóstico y ajuste del robot.
- **SmartConfig + mDNS**: Buena solución para configurar WiFi en campo sin hardcodear redes.

### Áreas de Mejora ⚠️

- **Credenciales hardcodeadas** *(corregido en este PR)*: Las credenciales de WiFi y MQTT estaban en texto plano en el código fuente, representando un riesgo de seguridad. Se han movido a Kconfig.
- **`ejecucionE1` y `ejecucionE2` son casi idénticas**: Los casos 0–17 son copia exacta entre ambas estrategias. Extraer la lógica común a un método privado reduciría el código duplicado significativamente.
- **La máquina de estados es frágil ante extensiones**: Usar enteros desnudos para `modo` y un `switch` de 24 casos hace difícil añadir nuevos estados. Un `enum class` mejoraría la legibilidad y detectaría errores en tiempo de compilación.
- **`mutex` como variable global en `main.cpp`**: Es accesible desde cualquier parte del código; sería más seguro encapsularlo dentro de la clase que lo necesite (`SensorLimite`).
- **Sin manejo de errores en la inicialización de periféricos**: Si un sensor falla al inicializarse, el código continúa silenciosamente. Verificar y reportar estos errores haría el sistema más robusto.
- **`test_motores.cpp.bak`**: Archivo de copia de seguridad no eliminado del repositorio.

---

## Estructura de Archivos

```
├── src/
│   ├── main.cpp              # Punto de entrada, inicialización de tareas
│   ├── MaquinaEstados.*      # Lógica de combate (máquina de estados)
│   ├── ControlMotores.*      # Control PWM de motores
│   ├── SensorLimite.*        # Sensores de borde (color)
│   ├── SensorRival.h         # Interfaz abstracta para sensores de rival
│   ├── SensorUltra.*         # Implementación ultrasónica
│   ├── SensorTof.*           # Implementación ToF (VL53L)
│   ├── Multiplexor.*         # Multiplexor I2C (TCA9548A)
│   ├── Wifi.*                # Gestión WiFi (STA + SmartConfig + mDNS)
│   ├── Mqtt.*                # Cliente MQTT
│   ├── Telemetria.*          # Publicación de telemetría
│   ├── Nvs.*                 # Abstracción NVS
│   ├── Ota.*                 # Actualizaciones OTA
│   ├── DatosT.h              # Estructura de datos de telemetría
│   ├── rgb.h                 # Control LED RGB
│   └── Kconfig.projbuild     # Configuración de credenciales (Kconfig)
├── lib/
│   └── Musica/               # Librería de melodías (buzzer)
├── platformio.ini            # Configuración de entornos PlatformIO
├── partitions.csv            # Tabla de particiones flash
├── sdkconfig.defaults        # Valores por defecto de Kconfig
├── sdkconfig.esp32           # Configuración generada para ESP32
└── sdkconfig.esp32-s3        # Configuración generada para ESP32-S3
```
