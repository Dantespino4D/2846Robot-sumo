# Robot Sumo Bidireccional - ESP32-S3

**Stack Tecnológico:**
![C/C++](https://img.shields.io/badge/C%2FC%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![FreeRTOS](https://img.shields.io/badge/FreeRTOS-20232A?style=for-the-badge&logo=freertos&logoColor=29B6F6) ![ESP-IDF v6](https://img.shields.io/badge/ESP--IDF_v6-E7352C?style=for-the-badge&logo=espressif&logoColor=white) ![STM32 Bare-Metal](https://img.shields.io/badge/STM32_Bare--Metal-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)

**Entorno de Desarrollo:**
![Arch Linux](https://img.shields.io/badge/Arch_Linux-1793D1?style=for-the-badge&logo=arch-linux&logoColor=white) ![Hyprland](https://img.shields.io/badge/Hyprland-00A489?style=for-the-badge&logo=hyprland&logoColor=white) ![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white) ![Ninja](https://img.shields.io/badge/Ninja-333333?style=for-the-badge&logo=ninja&logoColor=white) ![Neovim](https://img.shields.io/badge/Neovim-57A143?style=for-the-badge&logo=neovim&logoColor=white)

Este proyecto implementa el firmware completo y la integración de hardware de un robot de sumo competitivo de categoría Mini Sumo (10 cm x 10 cm, sin límite de altura). El diseño del robot es **bidireccional**, permitiéndole atacar y defenderse de manera eficaz sin necesidad de girar sobre su propio eje ante ataques por la retaguardia.

El sistema toma decisiones en tiempo real mediante una máquina de estados ejecutada sobre el RTOS FreeRTOS, procesando datos de múltiples sensores y controlando un sistema de tracción en las 4 ruedas. El código base ha sido migrado a **ESP-IDF v6**, utilizando una arquitectura modular basada en componentes y el sistema de construcción nativo `idf.py`.

### Características principales del Software

| Característica | Detalle |
|---|---|
| Microcontrolador | ESP32-S3 (Cerebro Táctico y Control) |
| Lenguajes & Frameworks | C/C++ nativo sobre ESP-IDF 6.x + FreeRTOS |
| Herramienta de build | idf.py |
| Arquitectura | Modular basada en componentes |
| Comunicación I2C | Arquitectura asíncrona basada en notificaciones de FreeRTOS para máxima eficiencia sin bloqueo de CPU. |
| Control de motores | PWM gestionando 4 motores mediante drivers independientes (DRV8871 vía MCPWM) |
| Conectividad | WiFi + MQTT + mDNS + SmartConfig + OTA |
| Robustez | Watchdog de tareas (WDT) y Timeout I2C por hardware. |

---

## Resultados del Prototipo V1

<img src="prototipoV1.jpeg" alt="Prototipo V1" width="600" />

El ensamblaje inicial V1 con tracción a 100 RPM sirvió como validación mecánica y de distribución de componentes. Sin embargo, al iniciar la fase de energización, el sistema presentó fallos de hardware críticos inmediatos:

- **Diagnóstico:** El uso de cableado no dedicado impidió un aislamiento adecuado entre las etapas de potencia de la batería y la lógica de control. Se detectaron riesgos inminentes de cortocircuito general antes de poder validar el firmware en movimiento.
- **Decisión Técnica:** En lugar de forzar parches sobre un hardware frágil, se abortó la etapa de pruebas dinámicas del primer prototipo por seguridad y fiabilidad.
- **Lección Aprendida:** Comprendí que el software más optimizado es inútil sobre un hardware inestable. Para competir de forma segura, es mandatorio diseñar circuitos impresos (PCBs) que aíslen el ruido de los motores, manejen corrientes altas sin riesgo térmico y aseguren la integridad de las señales.

---

## 🚀 Evolución a la Versión V2 (Arquitectura Híbrida en PCB)

A raíz de los hallazgos del primer prototipo, el proyecto pivotó hacia un diseño profesional en PCB y una arquitectura de hardware distribuida. Esta separación previene que la etapa de tracción comprometa al procesador principal:

- **Arquitectura Dual Inter-MCU:** Implementación de bus SPI de alta velocidad entre el ESP32-S3 y el STM32G474RET6, operando estrictamente por DMA con empaquetado de structs en C/C++.
- **Actualización de etapa de potencia:** Uso de drivers DRV8874, aprovechando el pin IPROPI para lectura analógica de corriente (muestreo continuo vía ADC con DMA en el STM32) para detección predictiva de stall y ajuste dinámico de torque.
- **Migración del regulador lógico:** A AP64203.
- **Corrección de interrupción:** Ruteo del pin de interrupción unificada de los VL53L1X hacia el GPIO1.
- **Delegación total del control de tracción:** Coprocesador STM32G474RET6, operando estrictamente en Bare-Metal (Low Layer). Uso de TIM1 para PWM y TIM6 con Lookup Tables para rampas de aceleración asíncronas.
- **Frenado activo de latencia cero (nanosegundos):** Ruteando los TCRT5000 directamente a los comparadores analógicos internos del STM32 vinculados al TIM1_BRK.
- **Telemetría diferida real:** Almacenamiento de logs en PSRAM en pleno combate y volcado asíncrono a LittleFS/MQTT comandado por botones físicos de Victoria/Derrota.
- **Inyección de configuración en caliente:** Despliegue de una red local (Access Point) generada por el ESP32 para inyección de configuraciones en caliente al NVS al entrar en modo de prueba.

---

## Especificaciones de Hardware (Actual)

* **Microcontrolador:** ESP32-S3
* **Tracción:** 4 Motores Pololu N20 de 100 RPM (reducción 298:1)
* **Control de Potencia:** 4 Drivers de motor DRV8871 independientes
* **Detección de Rival:** 2 Sensores ultrasónicos **HC-SR04**
* **Detección de Tatami:** 2 Sensores de color **TCS34725**
* **Expansión I2C:** Multiplexor **TCA9548A**
* **Alimentación:** Batería LiPo 2s 2200mAh 50C, gestionada por un BMS 2s de 20A.
* **Regulación:** Regulador **LM2596** para el sistema lógico.

---

## Optimización Real-Time

El sistema integra optimizaciones de bajo nivel para garantizar un rendimiento Hard Real-Time en combate:

1. **Reacción de Baja Latencia (Bypass de Software):** La detección de los bordes del tatami utiliza el módulo MCPWM Fault del ESP32-S3 para clavar los frenos a nivel de hardware, reduciendo significativamente la carga de CPU. Simultáneamente, una interrupción despierta a la Máquina de Estados asíncronamente mediante `xTaskNotifyWait`, asegurando una evasión rápida sin causar inanición a otras tareas.
2. **Concurrencia Lock-Free (Control de Motores):** La transferencia de setpoints de velocidad desde la estrategia hacia los generadores PWM se realiza mediante **Bit-Packing atómico** (`std::atomic<uint32_t>`). Esto evita el uso de bloqueos (Mutex/Spinlocks) en FreeRTOS.
3. **Empaquetado Estricto de Memoria:** El sistema de comunicación y telemetría utiliza estructuras con empaquetado forzado a 1 byte (`#pragma pack(push, 1)`) para prevenir corrupción de memoria por desalineación introducida por el compilador.

---

## Arquitectura del Software

El firmware utiliza al máximo las capacidades del ESP32-S3 mediante múltiples tareas FreeRTOS supervisadas por un **Task Watchdog (WDT)**. El sistema implementa una lógica de reinicio rápido en caso de fallo, omitiendo esperas innecesarias para reincorporarse al combate al instante.

```text
Core 0                         Core 1
──────────────────────         ──────────────────────
Sensores Ultra (Prio: 10)      Motores (Prio: 5)
Telemetria (Prio: 1)           Sensores TCS (Prio: 10)
Musica (Prio: 1)               Lógica / Combat (Prio: 2)
                               Interrupciones (Prio: 3)
```

### Módulos por Componentes

- **`core`** — El corazón del robot. Contiene la `MaquinaEstados`, el `GestorI2C`, la abstracción `Nvs` y todas las **Estrategias** de combate.
    - **Estrategias:** Implementa un **Patrón Strategy** con un puntero polimórfico (`estActual`) para ejecutar diferentes comportamientos en tiempo real.
    - **Memoria Táctica:** Sistema de memoria para la persecución predictiva del rival y anti-jitter.
    - **GestorI2C:** Arquitectura de bus con timeout de hardware de 5ms y autorrecuperación en caso de fallos.
- **`actuadores`** — Gestión de potencia.
    - **ControlMotores:** Control de los 4 motores DC mediante MCPWM del ESP32. Incluye una tarea dedicada (`tareaRampa`) que gestiona la aceleración.
    - **Multiplexor:** Driver para el TCA9548A.
- **`sensores`** — Drivers autogestionados.
    - **Interfaces:** `SensorLimite` y `SensorRival` permiten intercambiar hardware de forma transparente.
- **`comunicaciones`** — Stack de conectividad WiFi, MQTT, mDNS y OTA.
- **`ui`** — Componente para el control del LED RGB de manera segura.
- **`configuracion`** — Definición de pines y máscaras de bits (`eventos.h`).
- **`Musica`** — Tonos para feedback auditivo del robot.

---

## Modos de Operación

| Modo | Descripción | Activación |
|---|---|---|
| `0` | **Prueba / Telemetría** — Activa WiFi y MQTT. Permite monitoreo y ajustes en tiempo real. | Mantener presionado el botón al encender. |
| `1` | **Combate Autónomo** — Modo competitivo estricto. Transmisión inalámbrica desactivada para máximo rendimiento. | Encendido normal (Por defecto). |

---

## Parámetros Ajustables (NVS)

Las variables tácticas críticas se pueden ajustar remotamente:

| Clave NVS | Descripción |
|---|---|
| **Tiempos y Estrategia** | |
| `tiempos/retroceso` | Milisegundos de reversa al detectar borde. |
| `tiempos/evasion` | Duración de la maniobra de escape. |
| `tiempos/estrategia` | Algoritmo inicial seleccionado. |
| `tiempo_rampa` | Aceleración progresiva de los motores (ms). |
| **Velocidades (PWM)** | |
| `velocidad_nI` / `nD` | Velocidad normal (Búsqueda). |
| `velocidad_aI` / `aD` | Velocidad de ataque. |
| `velocidad_eI` / `eD` | Velocidad de evasión. |
| **Sensores y Sistema** | |
| `umbral_color` | Valor de referencia para detección de línea blanca. |
| `dist_max` | Rango máximo de detección del rival (cm). |

---

## Ecosistema de Herramientas

- 🐍 **`telemetria.py`**: Middleware en Python para análisis de datos y almacenamiento en base de datos local SQLite persistente. Extrae logs post-asalto para no penalizar el loop de control ni afectar la latencia del robot.
- 📜 **`commit.py`**: Script de automatización que inyecta el hash del commit para trazabilidad del firmware.
- 📦 **`full_codebase.sh`**: Utilidad para el mantenimiento y empaquetado del repositorio.

---

## Estructura del Proyecto

```text
├── components/
│   ├── actuadores/      # Motores y Multiplexor
│   ├── comunicaciones/  # WiFi, MQTT, OTA, mDNS
│   ├── configuracion/   # Pines y Eventos
│   ├── core/            # Maquina de estados, Estrategias y NVS
│   ├── sensores/        # Drivers Ultra, TCS
│   ├── ui/              # LED RGB (Interfaz centralizada)
│   └── Musica/          # Buzzer
├── main/
│   ├── main.cpp         # Punto de entrada e inicialización
│   └── idf_component.yml # Dependencias externas
├── partitions_s3.csv    # Tabla de particiones
├── sdkconfig            # Configuración activa del proyecto
└── CMakeLists.txt       # Script de construcción principal
```

---

## Compilación y Flasheo

```bash
# Limpiar y compilar
idf.py fullclean build

# Flashear y monitorear
idf.py -p [PUERTO] flash monitor
```
