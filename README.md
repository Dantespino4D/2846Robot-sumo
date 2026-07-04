# Robot Sumo Bidireccional - ESP32 / ESP32-S3

Este proyecto implementa el firmware completo y la integración de hardware de un robot de sumo competitivo de categoría Mini Sumo (10 cm x 10 cm, sin límite de altura). El diseño del robot es **bidireccional**, permitiéndole atacar y defenderse de manera eficaz sin necesidad de girar sobre su propio eje ante ataques por la retaguardia.

El sistema toma decisiones en tiempo real mediante una máquina de estados ejecutada sobre el RTOS FreeRTOS, procesando datos de múltiples sensores y controlando un sistema de tracción en las 4 ruedas. El código base ha sido migrado a **ESP-IDF v6**, utilizando una arquitectura modular basada en componentes y el sistema de construcción nativo `idf.py`.

### Características principales del Software

| Característica | Detalle |
|---|---|
| Microcontrolador | Híbrido Asimétrico: ESP32-S3 (Cerebro Táctico) + STM32G474RET6 (Músculo y Reflejos) |
| Framework | ESP-IDF 6.x + FreeRTOS (ESP32) / Bare-Metal LL (STM32) |
| Herramienta de build | idf.py (ESP32) / CMake + Ninja (STM32) |
| Arquitectura | Modular distribuida vía SPI DMA ("Bus de la Verdad") |
| Comunicación I2C | Arquitectura asíncrona basada en **DMA** y **Notificaciones Indexadas** de FreeRTOS para máxima eficiencia sin bloqueo de CPU. |
| Control de motores | PWM (MCPWM de ESP-IDF) gestionando 4 motores mediante drivers independientes |
| Conectividad | WiFi + MQTT + mDNS + SmartConfig + OTA |
| Telemetría | Streaming de estado en tiempo real al broker MQTT |
| Configuración | NVS (Non-Volatile Storage) en partición dedicada para parámetros ajustables en caliente |
| Robustez | Watchdog de tareas (WDT), Timeout I2C por hardware y validación CRC-8 por silicio. |
| Gestión de Memoria | Asignación Estática (FreeRTOS) y Ping-Pong Buffering (DMA STM32) para cero *data tearing*. |

---

## Especificaciones de Hardware

El proyecto está diseñado en dos fases de desarrollo integradas en el mismo repositorio para optimizar el rendimiento de combate:

### Prototipo (ESP32)
* **Tracción:** 4 Motores Pololu N20 de 100 RPM (reducción 298:1)
* **Control de Potencia:** 4 Drivers de motor DRV8871 independientes
* **Detección de Rival:** 2 Sensores ultrasónicos **HC-SR04**
* **Detección de Tatami:** 2 Sensores de color **TCS34725**
* **Expansión I2C:** Multiplexor **TCA9548A**
* **Alimentación:** Batería LiPo 2s 2200mAh 50C, gestionada por un BMS 2s de 20A.
* **Regulación:** Regulador **LM2596** para el sistema lógico.

### Versión Final Competitiva (ESP32-S3)
* **Tracción de Alta Velocidad:** Actualización a 4 Motores Pololu de 1000 RPM (reducción 50:1) para maximizar la velocidad de embestida.
* **Precisión Láser:** Sustitución de los sensores ultrasónicos por 6 sensores ToF **VL53L1X** (Láser) con direccionamiento dinámico mediante pines **XSHUT** dedicados.
* **Optimización del Campo de Visión (ROI):** Implementación de **Región de Interés (ROI)** por hardware para estrechar el haz láser a una franja horizontal de 16x4 SPADs. Al desplazar electrónicamente el centro de detección a la parte superior de la matriz, se elimina por completo la detección accidental del suelo (ruido), manteniendo un rango de visión horizontal amplio de ~27° para no perder al rival.
* **Coprocesador de Reflejos (STM32G474RET6):** Delega todo el control de los motores, rampas PWM y lectura de línea blanca (TCRT5000) a un microcontrolador secundario operando en Bare-Metal (Low Layer).
* **Bus de la Verdad (SPI + DMA):** Comunicación inter-procesos a alta velocidad donde el ESP32 (Maestro) transmite comandos y el STM32 (Esclavo) los recibe vía DMA, sin intervención de la CPU, validando la integridad mediante **CRC-8 por hardware**.
* **Gestión de Eventos:** Uso de los pines **INT** de los ToF. Gracias a la matriz IO_MUX del ESP32-S3, se dedica un pin físico independiente para la interrupción de cada uno de los 6 sensores, eliminando por completo la latencia de hacer "polling" vía I2C.
* **Evasión Pura en Silicio:** El STM32 enruta los sensores de línea y el "Kill Switch" a comparadores análogos vinculados directamente al `TIM1_BRK`, cortando la tracción en ~22 nanosegundos (0 µs de CPU).
* **Monitor del Sistema:** Lectura en tiempo real del voltaje de la batería y monitorización de corriente utilizando el ADC.
* **Almacenamiento Expandido:** Configuración de mapa de particiones de **4MB** por aplicación para soportar el framework v6, logs extensos y futuras expansiones de firmware.

---

## Arquitectura Híbrida Asimétrica (El Bus de la Verdad)

Para operar con estabilidad a 1000 RPM (donde 1 ms de latencia equivale a 2 mm de avance a ciegas), la arquitectura se divide en dos dominios cognitivos:

1. **Cerebro Táctico (ESP32-S3):** Se encarga exclusivamente de la visión espacial (leer 6 ToFs por I2C a 400kHz), inteligencia artificial (TinyML), rutinas de persecución, telemetría MQTT y control de la máquina de estados maestro.
2. **Sistema Nervioso (STM32G474):** Se encarga de la fuerza bruta y los reflejos instintivos. Modula el PWM de los motores y vigila la línea blanca.

**Integridad del Bus (SPI DMA Ping-Pong):**
El STM32 utiliza *Double Buffering* (Ping-Pong) a nivel de interrupción DMA. Esto aísla los espacios de memoria y evita el *Data Tearing* cuando el ESP32 inyecta un nuevo paquete asíncrono mientras el STM32 procesa el anterior. Todos los paquetes en tránsito se verifican mediante la unidad de hardware **CRC** del STM32 (polinomio 0x07, sin latencia de CPU).

---

## Optimización Real-Time (Safety Rollback)

El sistema integra optimizaciones de bajo nivel para garantizar un rendimiento Hard Real-Time en combate:

1. **Reacción de Latencia Cero (Bypass por Hardware e Interrupciones):** La detección de la línea blanca (TCRT5000) no depende de bucles de software (polling). Utiliza el módulo MCPWM Fault del ESP32 para clavar los frenos a nivel de silicio puros (0 µs de carga de CPU). Simultáneamente, una interrupción externa (EXTI) despierta a la Máquina de Estados asíncronamente mediante `xTaskNotifyWait`, asegurando una maniobra de evasión instantánea sin causar inanición (*starvation*) a tareas de baja prioridad como la telemetría.
2. **Concurrencia Lock-Free (Control de Motores):** La transferencia de setpoints de velocidad desde la estrategia hacia los generadores PWM se realiza mediante **Bit-Packing atómico** (`std::atomic<uint32_t>`). Esto evita el uso de bloqueos (Mutex/Spinlocks) en FreeRTOS, garantizando que las comunicaciones críticas de altísima velocidad (como la lectura I2C a 400kHz de los láseres ToF) nunca sean interrumpidas o cegadas por las actualizaciones de los motores.
3. **Empaquetado Estricto de Memoria:** El sistema de telemetría y comunicación inter-procesos utiliza estructuras con empaquetado forzado a 1 byte (`#pragma pack(push, 1)`). Esto previene corrupción de memoria por desalineación (*padding*) introducida por el compilador cruzado Xtensa, asegurando transferencias seguras y predecibles en la red de sensores.

---

## Arquitectura del Software

El firmware utiliza al máximo las capacidades del microcontrolador mediante múltiples tareas FreeRTOS supervisadas por un **Task Watchdog (WDT)**, asignando procesos críticos y periféricos a núcleos específicos para evitar bloqueos. El sistema implementa una **lógica de reinicio inteligente**: si el Watchdog provoca un reinicio, el robot detecta el estado y omite la espera de seguridad de 5 segundos para reincorporarse al combate instantáneamente.

```text
Core 0                         Core 1
──────────────────────         ──────────────────────
Sensores ToF (Prio: 10)        Motores (Prio: 5)
Telemetria (Prio: 1)           Sensores TCRT (Prio: 10)
Musica (Prio: 1)               Lógica / Combat (Prio: 2)
                               Sensores TCS (Prio: 3)
                               Interrupciones (Prio: 3)
```

### Módulos por Componentes

- **`core`** — El corazón del robot. Contiene la `MaquinaEstados`, el `GestorI2C`, la abstracción `Nvs` y todas las **Estrategias** de combate. 
    - **Estrategias:** Implementa un **Patrón Strategy** con un puntero polimórfico (`estActual`) para ejecutar diferentes comportamientos (E1, E2, Prototipo) en tiempo real.
    - **Memoria Táctica:** Sistema de **memoria a corto plazo (Zero-Order Hold)** de 10 estados y un sistema de **memoria a largo plazo** para la persecución predictiva del rival.
    - **GestorI2C:** Encargado de la robustez del hardware. Implementa un sistema de **auto-recuperación (self-healing)** y una arquitectura de **bus con colas de transacciones DMA** para operaciones no bloqueantes.
- **`actuadores`** — Gestión de potencia. 
    - **ControlMotores:** Control de los 4 motores DC mediante **MCPWM**. Incluye una tarea dedicada (`tareaRampa`) que gestiona la aceleración mediante una **Look-Up Table (LUT)** de 50 pasos.
    - **Multiplexor:** Driver para el TCA9548A usado en el prototipo.
- **`sensores`** — Drivers autogestionados que actualizan el `EventGroup` global de forma autónoma.
    - **SensorTof:** Gestión de los 6 VL53L1X con remapeo dinámico, configuración ROI y **lectura asíncrona mediante DMA**. Utiliza **Notificaciones Indexadas de FreeRTOS** (Buzón 1) para despertar la tarea solo cuando los datos están listos en RAM interna, evitando colisiones con interrupciones físicas.
    - **Interfaces:** `SensorLimite` y `SensorRival` permiten el intercambio transparente de hardware.
- **`comunicaciones`** — Stack de conectividad WiFi, MQTT (vía `espressif/mqtt`), mDNS y actualizaciones OTA. El JSON de telemetría se auto-adapta dinámicamente según el hardware detectado.
- **`ui`** — Componente centralizado para el control del LED RGB, garantizando acceso seguro desde múltiples módulos y evitando conflictos de recursos.
- **`configuracion`** — Definición centralizada del hardware map (`pines.h`), jerarquía de bits (`eventos.h`) y constantes globales.
- **`Musica`** — Generación de tonos y melodías para feedback auditivo del robot.

---

## Modos de Operación

| Modo | Descripción | Activación |
|---|---|---|
| `0` | **Prueba / Telemetría** — Activa WiFi y MQTT. Permite monitoreo y ajustes en tiempo real. | Mantener presionado el botón al encender. |
| `1` | **Combate Autónomo** — Modo competitivo estricto. WiFi y loggers desactivados para máximo rendimiento de CPU. | Encendido normal (Por defecto). |

---

## Parámetros Ajustables (NVS)

Las variables tácticas críticas se pueden ajustar vía MQTT sin necesidad de usar cables ni recompilar:

| Clave NVS | Descripción |
|---|---|
| **Tiempos y Estrategia** | |
| `tiempos/retroceso` | Milisegundos de reversa al pisar la línea blanca. |
| `tiempos/evasion` | Duración de la maniobra de escape tras detectar un atasco (ms). |
| `tiempos/recta_star` | Avance en rutina de búsqueda tipo estrella. |
| `tiempos/giro_star` | Rotación en rutina de búsqueda tipo estrella. |
| `tiempos/estrategia` | Algoritmo inicial seleccionado (0 = EP, 1 = E1, 2 = E2). |
| `tof_corto_plazo` | Memoria de persistencia de detección (Anti-jitter). |
| `tof_largo_plazo` | Tiempo de búsqueda predictiva. |
| `tiempo_rampa` | Aceleración progresiva de los motores (ms). |
| **Velocidades (PWM)** | |
| `velocidad_nI` / `nD` | Velocidad normal (Búsqueda). |
| `velocidad_aI` / `aD` | Velocidad de ataque. |
| `velocidad_eI` / `eD` | Velocidad de evasión tras stall. |
| `velocidad_mI` / `mD` | Velocidad de maximo. |
| `velocidad_pI` / `pD` | Velocidad de pronunciado. |
| `velocidad_gI` / `gD` | Velocidad de giro sobre eje. |
| `velocidad_hI` / `hD` | Velocidad de huida del borde. |
| **Sensores y Sistema** | |
| `u_stall` | Umbral de corriente para detectar atasco (Float en Amperios). |
| `t_stall` | Tiempo de confirmación de corriente alta para activar stall (ms). |
| `umbral_color` | Valor de referencia para detección de línea blanca. |
| `dist_max` | Rango máximo de detección del rival (cm/mm). |

---

## Sistema de Máscaras de Bits

El proyecto implementa un sofisticado sistema de detección basado en máscaras de bits (`eventos.h`) que permite evaluar combinaciones de sensores con eficiencia máxima:

```cpp
// Sensores de línea (borde del tatami)
MASK_LIM_A (AI, AD), MASK_LIM_B (BI, BD) → MASK_COLOR

// Sensores ToF frontales (dirección A)
BIT_TOF_AI, BIT_TOF_AC, BIT_TOF_AD → MASK_TOF_A

// Sensores ToF traseros (dirección B)
BIT_TOF_BI, BIT_TOF_BC, BIT_TOF_BD → MASK_TOF_B

// Sensores ultrasónicos (prototipo)
BIT_ULTRA_A, BIT_ULTRA_B     → MASK_ULTRA
```

---

## Fortalezas del Diseño

* **Detección de Stall y Maniobra de Evasión:** El sistema detecta bloqueos mecánicos mediante el ADC. Al superar el umbral `u_stall` durante `t_stall`, se ejecuta una maniobra de escape atómica.
* **Lógica de Huida Especializada (Anti-Borde):** Comandos tácticos dedicados (`HUIR_A`, `HUIR_B`) que gestionan de forma atómica el desbloqueo de motores para salir de la línea blanca a velocidad de escape configurable.
* **Sincronización No Bloqueante (Task Notifications):** Optimización de la latencia entre interrupciones (ISR) y tareas de procesamiento mediante `xTaskNotifyFromISR` para una respuesta instantánea a los bordes.
* **Optimización de Bus MCPWM ("dirty check"):** Actualización de los comparadores solo cuando existe una variación real en la velocidad calculada, minimizando el tráfico en el bus de periféricos.
* **Determinismo en Rampas:** El uso de `vTaskDelayUntil` garantiza que la rampa se ejecute con una frecuencia precisa y constante.
* **Sincronización de Memoria Crítica:** Uso de `portENTER_CRITICAL` para evitar condiciones de carrera entre la tarea de rampa y la máquina de estados.
* **Sistema Anti-Jitter (Zero-Order Hold):** Matriz de memorias a corto plazo para mitigar falsos positivos de los ToF en entornos de alta velocidad.
* **Diseño Bidireccional:** Ventaja táctica que permite invertir el sentido de ataque instantáneamente sin girar sobre el propio eje.

---

## Ecosistema de Herramientas

- **`telemetria.py`**: Middleware en Python que gestiona una base de datos local SQLite persistente y realiza el reenvío de datos.
- **`commit.py`**: Script de automatización que inyecta el hash del commit para trazabilidad del firmware.
- **`full_codebase.sh`**: Utilidad para el mantenimiento y empaquetado del repositorio.

---

## Estructura del Proyecto

```text
├── components/
│   ├── actuadores/      # Motores y Multiplexor
│   ├── comunicaciones/  # WiFi, MQTT, OTA, mDNS
│   ├── configuracion/   # Pines y Eventos
│   ├── core/            # Maquina de estados, Estrategias y NVS
│   ├── sensores/        # Drivers ToF, TCRT, TCS, Ultra
│   ├── ui/              # LED RGB (Interfaz centralizada)
│   └── Musica/          # Buzzer
├── main/
│   ├── main.cpp         # Punto de entrada e inicialización
│   └── idf_component.yml # Dependencias externas (MQTT, cJSON, etc.)
├── partitions_s3.csv    # Tabla de particiones (4MB por App)
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
