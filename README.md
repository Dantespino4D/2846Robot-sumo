# Robot Sumo Bidireccional - ESP32 / ESP32-S3 (ESP-IDF v6)

Este proyecto implementa el firmware completo y la integración de hardware de un robot de sumo competitivo de categoría Mini Sumo (10 cm x 10 cm, sin límite de altura). El diseño del robot es **bidireccional**, permitiéndole atacar y defenderse de manera eficaz sin necesidad de girar sobre su propio eje ante ataques por la retaguardia.

El sistema toma decisiones en tiempo real mediante una máquina de estados ejecutada sobre el RTOS FreeRTOS, procesando datos de múltiples sensores y controlando un sistema de tracción en las 4 ruedas. El código base ha sido migrado a **ESP-IDF v6**, utilizando una arquitectura modular basada en componentes y el sistema de construcción nativo `idf.py`.

### Características principales del Software

| Característica | Detalle |
|---|---|
| Microcontrolador | ESP32 / ESP32-S3 (Unificado por hardware map) |
| Framework | ESP-IDF 6.x + FreeRTOS |
| Herramienta de build | idf.py (Nativo) |
| Arquitectura | Modular basada en Componentes (ESP-IDF Components) |
| Control de motores | PWM (MCPWM de ESP-IDF) gestionando 4 motores mediante drivers independientes |
| Conectividad | WiFi + MQTT + mDNS + SmartConfig + OTA |
| Telemetría | Streaming de estado en tiempo real al broker MQTT |
| Configuración | NVS (Non-Volatile Storage) en partición dedicada para parámetros ajustables en caliente |
| Robustez | Watchdog de tareas (WDT) y auto-recuperación de bus I2C con timeout de hardware |
| Gestión de Memoria | **Asignación Estática de FreeRTOS** para todas las tareas críticas, eliminando riesgos de fragmentación del heap y garantizando estabilidad a largo plazo. |

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
* **Gestión de Eventos:** Uso de los pines **INT** de los ToF. Gracias a la matriz IO_MUX del ESP32-S3, se dedica un pin físico independiente para la interrupción de cada uno de los 6 sensores, eliminando por completo la latencia de hacer "polling" vía I2C.
* **Detección de Stall (Atasco):** Implementación de detección por hardware mediante la medición de caída de voltaje en una resistencia shunt leída por el ADC (**GPIO_NUM_1**). Esto permite identificar bloqueos mecánicos o empujes del rival de forma instantánea.
* **Eficiencia I2C:** Eliminación del multiplexor TCA9548A en favor del bus compartido de los ToF, simplificando el cableado.
* **Detección Infrarroja:** Implementación de sensores **TCRT5000** mediante interrupciones de hardware para una respuesta instantánea al borde del tatami.
* **Alimentación Optimizada:** Sustitución del regulador LM2596 por el módulo **Mini 360 (MP2307)**, logrando mayor eficiencia energética y un diseño más compacto.
* **Monitor del Sistema:** Lectura en tiempo real del voltaje de la batería y monitorización de corriente utilizando la API ADC Oneshot de ESP-IDF.
* **Almacenamiento Expandido:** Configuración de mapa de particiones de **4MB** por aplicación para soportar el framework v6, logs extensos y futuras expansiones de firmware.

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
    - **GestorI2C:** Encargado de la robustez del hardware. Implementa un sistema de **auto-recuperación (self-healing)** que detecta bloqueos del bus y configura un **timeout de hardware (64000 ticks)**.
- **`actuadores`** — Gestión de potencia. 
    - **ControlMotores:** Control de los 4 motores DC mediante **MCPWM**. Incluye una tarea dedicada (`tareaRampa`) que gestiona la aceleración mediante una **Look-Up Table (LUT)** de 50 pasos.
    - **Multiplexor:** Driver para el TCA9548A usado en el prototipo.
- **`sensores`** — Drivers autogestionados que actualizan el `EventGroup` global de forma autónoma.
    - **SensorTof:** Gestión de los 6 VL53L1X con remapeo dinámico, configuración ROI y **lectura de ráfaga (burst read)** de bajo nivel al registro `0x0089`.
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
BIT_LIM_A, BIT_LIM_B           → MASK_COLOR

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
