# Robot Sumo

Firmware para robot de sumo autónomo basado en **ESP32 / ESP32-S3** con ESP-IDF y PlatformIO.

---

## Descripción del Proyecto

Este proyecto implementa el firmware completo y la integración de hardware de un robot de sumo competitivo de categoría Mini Sumo (10 cm x 10 cm, sin límite de altura). El diseño del robot es **bidireccional**, permitiéndole atacar y defenderse de manera eficaz sin necesidad de girar sobre su propio eje ante ataques por la retaguardia.

El sistema toma decisiones en tiempo real mediante una máquina de estados ejecutada sobre el RTOS FreeRTOS, procesando datos de múltiples sensores y controlando un sistema de tracción en las 4 ruedas.

### Características principales del Software

| Característica | Detalle |
|---|---|
| Microcontrolador | ESP32 / ESP32-S3 (seleccionable por entorno) |
| Framework | ESP-IDF 5.x + FreeRTOS |
| Herramienta de build | PlatformIO |
| Control de motores | PWM (LEDC de ESP-IDF) gestionando 4 motores mediante drivers independientes |
| Conectividad | WiFi + MQTT + mDNS + SmartConfig + OTA |
| Telemetría | Streaming de estado en tiempo real al broker MQTT |
| Configuración | NVS (Non-Volatile Storage) para parámetros ajustables en caliente |

---

## Especificaciones de Hardware

El proyecto está diseñado en dos fases de desarrollo para optimizar el rendimiento de combate:

### Fase 1: Prototipo Actual
* **Tracción:** 4 Motores Pololu N20 de 100 RPM (reducción 298:1)
* **Control de Potencia:** 4 Drivers de motor DRV8871 independientes
* **Detección de Rival:** 2 Sensores ultrasónicos HC-SR04
* **Detección de Tatami:** 2 Sensores de color TCS34725
* **Expansión I2C:** Multiplexor TCA9548A
* **Alimentación:** Batería LiPo 2s 2200mAh 50C, gestionada por un BMS 2s de 20A y un regulador LM2596
* **Cableado:** Alambre AWG 24 para señales lógicas y cable de silicona AWG 18 para potencia.

### Fase 2: Versión Final Competitiva
* **Tracción de Alta Velocidad:** Actualización a 4 Motores Pololu de 1000 RPM (reducción 50:1) para maximizar la velocidad de embestida.
* **Precisión Láser:** Sustitución de los sensores ultrasónicos por sensores ToF (Time-of-Flight) **VL53L0X** para una lectura de distancia al rival más rápida, precisa e inmune a interferencias acústicas.

---

## Arquitectura del Software

El firmware utiliza al máximo las capacidades del ESP32 mediante múltiples tareas FreeRTOS, asignando procesos críticos y periféricos a núcleos específicos para evitar bloqueos:

```text
Core 0                         Core 1
──────────────────────         ──────────────────────
SensorUltra (Prio: 2)          Motores (Prio: 5)
Telemetria (Prio: 1)           SensorColor (Prio: 3)
Musica (Prio: 1)               Robot / Lógica (Prio: 2)

### Módulos principales

- **`MaquinaEstados`** — Núcleo táctico del robot. Utiliza un **Patrón Strategy** con un puntero polimórfico (`estActual`) para ejecutar diferentes comportamientos en tiempo real. Gestiona la limpieza de memorias por tiempo (`tiempo()`) de manera global. Soporta múltiples estrategias seleccionables desde NVS (prototipo, E1, E2). Implementa un avanzado sistema de **memoria a corto plazo (Zero-Order Hold)** de 10 estados para evitar "tartamudeos" por el parpadeo o multiplexado de los sensores, y un sistema de **memoria a largo plazo** para la persecución ciega y predictiva del rival.
- **`Estrategias`** — Sistema modular de combate que utiliza máscaras de bits para decisiones de alta velocidad:
    - `EstrategiaBase.h`: Interfaz abstracta que define el contrato de `seleccion()` y `ejecucion()`.
    - `EstrategiaEstandar.*`: Clase intermedia que implementa la lógica común de los 6 sensores ToF y sensores de línea para evitar duplicidad de código. Utiliza una jerarquía de máscaras para filtrar primero por zonas (MASK_TOF_A, MASK_TOF_B, MASK_COLOR) y luego evaluar combinaciones específicas de sensores.
    - `EstrategiaPrototipo.*`: Estrategia legacy para el prototipo con sensores ultrasónicos (HC-SR04). Aplica el mismo patrón de máscaras de bits (MASK_ULTRA, BIT_ULTRA_A, BIT_ULTRA_B) para mantener consistencia arquitectónica.
    - `Estrategia1.*` y `Estrategia2.*`: Estrategias de combate específicas que heredan de EstrategiaEstandar, permitiendo variaciones tácticas sin duplicar código de sensores.
- **`eventos.h`** — Definición centralizada de la jerarquía de bits y máscaras de acción. Permite una lógica de decisión de alta eficiencia mediante el filtrado de zonas (Frontal, Trasera, Borde) y combinaciones de precisión. La arquitectura de máscara jerárquica permite evaluar grupos de sensores en un solo ciclo de CPU antes de descender a combinaciones específicas, optimizando la toma de decisiones en tiempo real.
- **`ControlMotores`** — Abstracción para el control PWM de los 4 motores DC. Define comandos estratégicos de alto nivel: direcciones, ataques directos, giros pronunciados y velocidad máxima.
- **`SensorLimite`** — Lectura en hilo secundario de los sensores de color TCS34725 para evadir el borde blanco del dohyo.
- **`SensorRival`** — Interfaz abstracta diseñada para facilitar la migración de los ultrasónicos HC-SR04 a los sensores ToF VL53L0X sin alterar la lógica superior.
- **`SensorTof`** — Gestión de los 6 sensores de tiempo de vuelo mediante multiplexación I2C para una visión de 360 grados.
- **`Wifi` / `Mqtt` / `Telemetria`** — Stack de conectividad que publica el estado completo del robot (lecturas, estados, hardware) al broker MQTT para análisis y telemetría de pruebas.
- **`Nvs`** — Capa de abstracción sobre la memoria NVS del ESP-IDF.

---

## Modos de Operación

El robot configura su modo operativo al arranque utilizando el botón de interfaz y la memoria NVS:

| Modo | Descripción | Activación |
|---|---|---|
| `0` | **Prueba / Telemetría** — Activa WiFi y MQTT. Permite monitoreo y ajustes en tiempo real. | Mantener presionado el botón al encender. |
| `1` | **Combate Autónomo** — Modo competitivo estricto. WiFi y loggers desactivados para máximo rendimiento de CPU. | Encendido normal (Por defecto). |

En el modo de combate, al presionar el botón de inicio se calibran los sensores de piso, se inician los 5 segundos reglamentarios de espera y comienza la rutina de búsqueda.

---

## Parámetros Ajustables (NVS)

Las variables tácticas críticas se pueden ajustar vía MQTT sin necesidad de usar cables ni recompilar:

| Clave NVS | Descripción |
|---|---|
| `tiempos/ataque_ciego` | Tiempo de empuje sostenido tras perder contacto visual con el rival. |
| `tiempos/retroceso` | Milisegundos de reversa al pisar la línea blanca. |
| `tiempos/recta_star` | Avance en rutina de búsqueda tipo estrella. |
| `tiempos/giro_star` | Rotación en rutina de búsqueda tipo estrella. |
| `tiempos/estrategia` | Algoritmo inicial seleccionado (0 = EP, 1 = E1, 2 = E2). |

---

## Sistema de Máscaras de Bits

El proyecto implementa un sofisticado sistema de detección basado en máscaras de bits que permite evaluar combinaciones de sensores con eficiencia máxima:

### Jerarquía de Bits (eventos.h)

```cpp
// Sensores de línea (borde del tatami)
BIT_SC_1, BIT_SC_2           → MASK_COLOR

// Sensores ToF frontales (dirección A)
BIT_TOF_AI, BIT_TOF_AC, BIT_TOF_AD → MASK_TOF_A
  ├─ MASK_TOF_1_2 (AI + AC)
  ├─ MASK_TOF_1_3 (AI + AD)
  └─ MASK_TOF_2_3 (AC + AD)

// Sensores ToF traseros (dirección B)
BIT_TOF_BI, BIT_TOF_BC, BIT_TOF_BD → MASK_TOF_B
  ├─ MASK_TOF_4_5 (BI + BC)
  ├─ MASK_TOF_4_6 (BI + BD)
  └─ MASK_TOF_5_6 (BC + BD)

// Sensores ultrasónicos (prototipo)
BIT_ULTRA_A, BIT_ULTRA_B     → MASK_ULTRA
```

### Flujo de Decisión Optimizado

El algoritmo de selección de estado utiliza un **filtrado jerárquico en cascada**:

1. **Prioridad Máxima:** `MASK_COLOR` — Detectar el borde del dohyo tiene máxima prioridad
2. **Memorias de Color:** Evaluar si se está alejando del borde
3. **Zona Frontal:** `MASK_TOF_A` o `MASK_ULTRA` (prototipo) — Evaluar combinaciones específicas
4. **Zona Trasera:** `MASK_TOF_B` — Evaluar si el rival está por detrás
5. **Memorias Temporales:** Estados de persecución ciega (corto y largo plazo)
6. **Estado por Defecto:** Rutina de búsqueda tipo estrella

Esta arquitectura permite que el procesador filtre **grupos completos de sensores en un solo ciclo de CPU** antes de descender a combinaciones específicas, reduciendo dramáticamente el tiempo de respuesta de la máquina de estados.

### Ventajas de este Diseño

* **Performance:** Una operación AND bit a bit (`noti & MASK_TOF_A`) evalúa 3 sensores simultáneamente
* **Legibilidad:** Las máscaras tienen nombres descriptivos que documentan el código
* **Escalabilidad:** Agregar nuevos sensores solo requiere definir bits y actualizar máscaras
* **Consistencia:** Todas las estrategias comparten las mismas definiciones de `eventos.h`

---

## Fortalezas del Diseño

* **Arquitectura de Estrategias Modulares:** El uso del Patrón Strategy permite crear nuevas tácticas de combate (ej. flanqueo, evasión, estrella) simplemente heredando de `EstrategiaEstandar`. Esto aísla el código de cada estrategia, facilitando el debug y permitiendo cambios en caliente sin riesgo de afectar otras tácticas.
* **Jerarquía de Máscaras de Bits:** La implementación de un mapa de eventos centralizado en `eventos.h` permite que la lógica de decisión sea extremadamente rápida y legible. El procesador puede filtrar zonas completas (ej. "¿Hay algo adelante?") en un solo ciclo de instrucción antes de evaluar combinaciones de precisión.
* **Abstracción Orientada a Hardware Evolutivo:** El uso de herencia y polimorfismo en `SensorRival` facilita la transición planificada a los sensores ToF VL53L0X.
* **Diseño Bidireccional:** Aporta una ventaja táctica inmensa, ya que la máquina de estados puede simplemente invertir motores para atacar a un rival trasero sin consumir tiempo valioso en girar.
* **Arquitectura FreeRTOS:** La segregación de la lógica del robot, la lectura del ADC/I2C de sensores de color y la interrupción de motores en distintas tareas priorizadas asegura tiempos de respuesta de milisegundos en combate.
* **Potencia Desacoplada:** El uso de 4 drivers DRV8871 permite aprovechar la máxima corriente de pico por cada rueda de manera individual, previniendo cuellos de botella térmicos frente al empuje extremo.
* **Sistema Anti-Jitter (Zero-Order Hold):** La máquina de estados procesa las lecturas multiplexadas de los sensores a través de una matriz de memorias a corto plazo. Esto permite que el robot mantenga una fluidez perfecta de ataque y previene tirones en los motores si se pierde una lectura de sensor por milisegundos.

---

## Estructura de Archivos

```text
├── src/
│   ├── actuadores/           # Control PWM y MUX
│   │   ├── ControlMotores.*  # Control PWM de motores
│   │   ├── Multiplexor.*     # Multiplexor I2C (TCA9548A)
│   │   └── rgb.h             # Control LED RGB
│   ├── comunicaciones/       # Conectividad y telemetría
│   │   ├── Mqtt.*            # Cliente MQTT
│   │   ├── Ota.*             # Actualizaciones OTA
│   │   ├── Telemetria.*      # Publicación de telemetría
│   │   └── Wifi.*            # Gestión WiFi (STA + SmartConfig + mDNS)
│   ├── configuracion/        # Variables y hardware map
│   │   ├── configuracion.*   # Utilidades de configuración
│   │   ├── eventos.h         # Definición de bits y máscaras
│   │   └── pines.h           # Mapa de pines
│   ├── core/                 # Lógica base
│   │   ├── DatosT.h          # Estructura de datos de telemetría
│   │   ├── MaquinaEstados.*  # Gestor de estados y tiempos
│   │   └── Nvs.*             # Abstracción NVS
│   ├── estrategias/          # Lógicas de combate
│   │   ├── Estrategia1.*     # Estrategia de combate 1
│   │   ├── Estrategia2.*     # Estrategia de combate 2
│   │   ├── EstrategiaBase.h  # Interfaz de estrategias
│   │   ├── EstrategiaEstandar.* # Lógica de sensores compartida
│   │   └── EstrategiaPrototipo.* # Estrategia inicial (Legacy)
│   ├── sensores/             # Hardware de medición
│   │   ├── SensorLimite.*    # Sensores de borde (color)
│   │   ├── SensorRival.h     # Interfaz abstracta para sensores de rival
│   │   ├── SensorTof.*       # Implementación ToF (VL53L)
│   │   └── SensorUltra.*     # Implementación ultrasónica
│   ├── main.cpp              # Punto de entrada, inicialización de tareas
│   ├── Kconfig.projbuild     # Configuración de credenciales (Kconfig)
│   └── idf_component.yml     # Dependencias de componentes (espp)
├── lib/
│   └── Musica/               # Librería de melodías (buzzer)
├── platformio.ini            # Configuración de entornos PlatformIO
├── partitions.csv            # Tabla de particiones flash
├── sdkconfig.defaults        # Valores por defecto de Kconfig
├── sdkconfig.esp32           # Configuración generada para ESP32
└── sdkconfig.esp32-s3        # Configuración generada para ESP32-S3
```
