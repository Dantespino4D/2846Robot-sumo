# Robot Sumo Bidireccional - ESP32 / ESP32-S3

Este proyecto implementa el firmware completo y la integración de hardware de un robot de sumo competitivo de categoría Mini Sumo (10 cm x 10 cm, sin límite de altura). El diseño del robot es **bidireccional**, permitiéndole atacar y defenderse de manera eficaz sin necesidad de girar sobre su propio eje ante ataques por la retaguardia.

El sistema toma decisiones en tiempo real mediante una máquina de estados ejecutada sobre el RTOS FreeRTOS, procesando datos de múltiples sensores y controlando un sistema de tracción en las 4 ruedas. El código base está **unificado**, permitiendo flashear tanto el prototipo como la versión final simplemente seleccionando el entorno en PlatformIO.

### Características principales del Software

| Característica | Detalle |
|---|---|
| Microcontrolador | ESP32 / ESP32-S3 (Unificado por entorno) |
| Framework | ESP-IDF 5.x + FreeRTOS |
| Herramienta de build | PlatformIO |
| Control de motores | PWM (LEDC de ESP-IDF) gestionando 4 motores mediante drivers independientes |
| Conectividad | WiFi + MQTT + mDNS + SmartConfig + OTA |
| Telemetría | Streaming de estado en tiempo real al broker MQTT |
| Configuración | NVS (Non-Volatile Storage) en partición dedicada para parámetros ajustables en caliente |
| Robustez | Watchdog de tareas (WDT) y auto-recuperación de bus I2C con timeout de hardware |
| Abstracción | Inyección de dependencias para sensores según el hardware (`final` flag) |
| Gestión de Memoria | **Asignación Estática de FreeRTOS** para todas las tareas críticas, eliminando riesgos de fragmentación del heap y garantizando estabilidad a largo plazo. |

## Estado del Desarrollo (WIP - Trabajo en Progreso)

> [!CAUTION]
> **Migración de Driver de Motores:** Actualmente el sistema se encuentra en una fase de transición del periférico **LEDC** al más avanzado **MCPWM** (Motor Control PWM). 
> * **Hardware:** Se ha implementado una parada de emergencia (Brake) por hardware vinculada al pin `TCRT_1`. Si este pin detecta una señal de falla (línea), el periférico bloqueará los motores instantáneamente sin intervención del software.
> * **Software:** El método `velocidad()` está siendo reescrito. En el commit actual, el robot **no responderá a comandos de movimiento** hasta que se complete el mapeo de los comparadores MCPWM.

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
* **Monitor del Sistema:** Lectura en tiempo real del voltaje de la batería y temperatura de 4 termistores utilizando la API ADC Oneshot de ESP-IDF.
* **Almacenamiento Expandido:** Configuración de flash de **16MB** en el ESP32-S3 para soportar logs extensos y futuras expansiones de firmware.

---

## Arquitectura del Software

El firmware utiliza al máximo las capacidades del ESP32 mediante múltiples tareas FreeRTOS supervisadas por un **Task Watchdog (WDT)**, asignando procesos críticos y periféricos a núcleos específicos para evitar bloqueos. El sistema implementa una **lógica de reinicio inteligente**: si el Watchdog provoca un reinicio, el robot detecta el estado y omite la espera de seguridad de 5 segundos para reincorporarse al combate instantáneamente. La inicialización del hardware es dinámica y se basa en la placa detectada durante el arranque.

```text
Core 0                         Core 1
──────────────────────         ──────────────────────
Sensores ToF (Prio: 10)        Motores (Prio: 5)
Telemetria (Prio: 1)           Sensores TCRT (Prio: 10)
Musica (Prio: 1)               Lógica / Combat (Prio: 2)
                               Sensores TCS (Prio: 3)
                               Interrupciones (Prio: 3)
```
### Módulos principales

- **`MaquinaEstados`** — Núcleo táctico del robot. Utiliza un **Patrón Strategy** con un puntero polimórfico (`estActual`) para ejecutar diferentes comportamientos en tiempo real. Recibe el flag `final` para ajustar su comportamiento según el hardware disponible. Implementa un avanzado sistema de **memoria a corto plazo (Zero-Order Hold)** de 10 estados y un sistema de **memoria a largo plazo** para la persecución ciega y predictiva del rival.
- **`Estrategias`** — Sistema modular de combate que utiliza máscaras de bits para decisiones de alta velocidad:
    - `EstrategiaBase.h`: Interfaz abstracta que define el contrato de `seleccion()` y `ejecucion()`.
    - `EstrategiaEstandar.*`: Clase intermedia que implementa la lógica común de los 6 sensores ToF y sensores de línea para evitar duplicidad de código. Utiliza una jerarquía de máscaras para filtrar primero por zonas (MASK_TOF_A, MASK_TOF_B, MASK_COLOR).
    - `EstrategiaPrototipo.*`: Estrategia para el prototipo con sensores ultrasónicos (HC-SR04) y TCS34725. Aplica el mismo patrón de máscaras de bits para mantener consistencia arquitectónica.
    - `Estrategia1.*` y `Estrategia2.*`: Estrategias de combate específicas que heredan de EstrategiaEstandar, permitiendo variaciones tácticas sin duplicar código de sensores.
- **`eventos.h`** — Definición centralizada de la jerarquía de bits y máscaras de acción. La arquitectura de máscara jerárquica permite evaluar grupos de sensores en un solo ciclo de CPU antes de descender a combinaciones específicas, optimizando la toma de decisiones en tiempo real.
- **`ControlMotores`** — Abstracción para el control PWM de los 4 motores DC. Define comandos estratégicos de alto nivel: direcciones, ataques directos, giros pronunciados y velocidad máxima.
- **`SensorLimite`** y **`SensorRival`** — Interfaces abstractas para sensores que permiten el intercambio transparente de hardware. Implementan un modelo de **procesamiento autónomo**, donde cada sensor gestiona su propia tarea de FreeRTOS para actualizar el `EventGroup` global, eliminando la necesidad de llamadas cíclicas desde el bucle principal.
- **`SensorTof`** — Gestión de los 6 sensores de tiempo de vuelo (**VL53L1X**). El firmware realiza un remapeo secuencial de direcciones I2C al arranque mediante los pines **XSHUT**, permitiendo la coexistencia de múltiples sensores en un solo bus sin colisiones. 
    - **Configuración ROI Dinámica:** Implementa un método `set_roi` que permite modificar físicamente el tamaño y la posición de la matriz de SPADs activa. Esto se utiliza para configurar una ventana de visión optimizada que ignora obstáculos estructurales y el brillo del tatami.
    - **Lectura de ráfaga (burst read):** Implementa una **lectura de ráfaga (burst read) de bajo nivel** al registro `0x0089`, permitiendo extraer en una sola transacción la distancia, el estado del sensor, la tasa de retorno de señal y el ruido de luz ambiente para validar la calidad de la detección en entornos con alta interferencia lumínica.
- **`GestorI2C`** — Módulo crítico encargado de la robustez del hardware. Implementa un sistema de **auto-recuperación (self-healing)** que detecta bloqueos del bus y configura un **timeout de hardware (64000 ticks)** para evitar colgar el procesador, garantizando que el robot no quede indefenso ante ruidos electromagnéticos.
- **`Nvs`** — Abstracción para el almacenamiento no volátil. Utiliza una **partición dedicada ("configuracion")** separada del NVS estándar del sistema para proteger los parámetros tácticos y asegurar su integridad ante actualizaciones o fallos del sistema.
- **`Telemetria`** — Stack de conectividad que publica el estado completo del robot de forma asíncrona. El JSON de telemetría se auto-adapta dinámicamente mediante el flag `final` según el hardware detectado.
    - **Middleware de Persistencia:** Utiliza un script externo (`telemetria.py`) que escucha vía MQTT, realiza una gestión de cola (batching) en una base de datos local SQLite y reenvía los datos mediante HTTP/HTTPS (con integración a una **base de datos externa** prevista próximamente).
    - **Salud de Sensores:** Reporta métricas de calidad para los ToF (estado, señal y ruido ambiental).
    - **Diagnóstico del Sistema:** El payload incluye telemetría profunda: memoria libre (`heap`), versión del firmware (`commit`), calidad de señal WiFi, ciclo de control, corriente en amperios (**corriente**) y estado de los motores (PWM y detección de `stall`).
- **`MonitorSistema`** — Módulo dedicado a la supervisión analógica. Realiza la lectura del voltaje de batería y la corriente de los motores (Stall). Implementa una conversión de alta precisión para 4 termistores mediante la **ecuación de Steinhart-Hart**, transformando lecturas de voltaje en grados Celsius para el monitoreo térmico del hardware.

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
| `velocidad_nI` / `nD` | Velocidad nominal (Búsqueda). |
| `velocidad_aI` / `aD` | Velocidad de ataque. |
| `velocidad_eI` / `eD` | Velocidad de evasión tras stall. |
| `velocidad_mI` / `mD` | Velocidad de maniobra. |
| `velocidad_pI` / `pD` | Velocidad de patrullaje. |
| `velocidad_gI` / `gD` | Velocidad de giro sobre eje. |
| **Sensores y Sistema** | |
| `u_stall` | Umbral de corriente para detectar atasco (Float en Amperios). |
| `t_stall` | Tiempo de confirmación de corriente alta para activar stall (ms). |
| `umbral_color` | Valor de referencia para detección de línea blanca. |
| `dist_max` | Rango máximo de detección del rival (cm/mm). |
| `modo` | Selección de modo de arranque (0=Prueba, 1=Combate). |
| `monitor` | Nivel de verbosidad del Monitor del Sistema. |

---

## Sistema de Máscaras de Bits

El proyecto implementa un sofisticado sistema de detección basado en máscaras de bits que permite evaluar combinaciones de sensores con eficiencia máxima:

### Jerarquía de Bits (eventos.h)

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

* **Detección de Stall y Maniobra de Evasión:** El sistema detecta bloqueos mecánicos mediante el ADC en tiempo real. Al superar el umbral `u_stall` durante un tiempo `t_stall`, la Máquina de Estados interrumpe la estrategia actual para ejecutar una **maniobra de evasión** (retroceso y giro rápido) definida por `tiempos/evasion`, garantizando que el robot no se queme ni quede atrapado en colisiones estáticas.
* **Sincronización No Bloqueante y Atómica (Task Notifications):** Se ha optimizado la comunicación entre las interrupciones (ISR) y las tareas de procesamiento. Para los sensores de línea (TCRT5000), se utiliza `xTaskNotifyFromISR` con transferencia de valor mediante máscaras de bits. Esto garantiza que el estado de los sensores se capture y entregue de forma atómica a la tarea, eliminando inconsistencias por cambios de estado rápidos y reduciendo la latencia al evitar lecturas redundantes del GPIO fuera de la ISR.
* **Arquitectura de Estrategias Modulares:** El uso del Patrón Strategy permite crear nuevas tácticas de combate simplemente heredando de `EstrategiaEstandar`.
* **Encapsulamiento de Sensores Autogestionados:** Cada clase de sensor gestiona su propia lectura, liberando al `main.cpp` de la gestión de hilos y garantizando una migración de hardware transparente.
* **Jerarquía de Máscaras de Bits:** Permite que la lógica de decisión sea extremadamente rápida y legible, filtrando grupos completos de sensores en un solo ciclo de CPU.
* **Diseño Bidireccional:** Aporta una ventaja táctica inmensa, ya que la máquina de estados puede simplemente invertir motores para atacar a un rival trasero sin consumir tiempo valioso en girar.
* **Sistema Anti-Jitter (Zero-Order Hold):** Procesa las lecturas de los sensores a través de una matriz de memorias a corto plazo para evitar ruidos en la toma de decisiones.
* **Determinismo y Estabilidad de Memoria:** Implementación de **Asignación Estática de FreeRTOS** (`xTaskCreateStatic`) para todas las tareas del sistema (`robot`, `motores`, `telemetría`, etc.). Esto elimina la dependencia del heap en tiempo de ejecución, previene errores de "Out of Memory" por fragmentación y asegura un comportamiento determinista, crítico para la fiabilidad en competencia.
* **Robustez de Interrupciones y Periféricos:** Mejora en la inicialización de los servicios de interrupción GPIO y gestión de handles de tareas por instancia, permitiendo una coexistencia más segura de múltiples sensores y evitando colisiones de hardware durante el arranque.

## Ecosistema de Herramientas

El proyecto incluye un conjunto de herramientas externas para la gestión de datos, automatización de compilación y configuración:

- **`telemetria.py`**: Middleware en Python encargado de suscribirse al broker MQTT, gestionar una base de datos local SQLite para evitar pérdida de datos y realizar el reenvío (batching) a un servidor central.
- **`commit.py`**: Script de automatización integrado en PlatformIO que inyecta el hash del commit actual en el código fuente durante la compilación, permitiendo la trazabilidad total del firmware.
- **`json-maestro.json` / `json-telemetria.json`**: Estructuras de definición para la configuración y mapeo de datos de telemetría entre el robot y el backend.
- **`full_codebase.sh`**: Utilidad para el mantenimiento y empaquetado del repositorio.

---

## Estructura del Proyecto

```text
├── src/
│   ├── actuadores/           # Control PWM y periféricos de salida
│   │   ├── ControlMotores.*  # Control PWM de los 4 motores
│   │   ├── Multiplexor.*     # TCA9548A (Prototipo)
│   │   └── rgb.h             # Control LED RGB
│   ├── comunicaciones/       # Conectividad y telemetría
│   │   ├── Mqtt.*            # Cliente MQTT
│   │   ├── Ota.*             # Actualizaciones OTA
│   │   ├── Telemetria.*      # Publicación de telemetría
│   │   └── Wifi.*            # Gestión WiFi (STA + SmartConfig + mDNS)
│   ├── configuracion/        # Variables y hardware map
│   │   ├── configuracion.*   # Utilidades de configuración
│   │   ├── eventos.h         # Definición de bits y máscaras
│   │   └── pines.h           # Mapa de hardware dual (ESP32/S3)
│   ├── core/                 # Lógica base y servicios
│   │   ├── DatosT.h          # Estructura de datos de telemetría
│   │   ├── GestorI2C.*       # Administración y salud del bus I2C
│   │   ├── MaquinaEstados.*  # Gestor de estados y tiempos
│   │   ├── MonitorSistema.*  # Supervisión analógica del sistema (ADC)
│   │   └── Nvs.*             # Abstracción NVS
│   ├── estrategias/          # Lógicas de combate
│   │   ├── Estrategia1.*     # Estrategia de combate 1
│   │   ├── Estrategia2.*     # Estrategia de combate 2
│   │   ├── EstrategiaBase.h  # Interfaz de estrategias
│   │   ├── EstrategiaEstandar.* # Lógica base para ToF
│   │   └── EstrategiaPrototipo.* # Lógica base para Ultrasonidos
│   ├── sensores/             # Hardware de medición
│   │   ├── SensorLimite.h    # Interfaz para detección de borde
│   │   ├── SensorRival.h     # Interfaz para detección de oponente
│   │   ├── SensorTcs.*       # Driver TCS34725 (Prototipo)
│   │   ├── SensorTcrt.*      # Driver TCRT5000 (Final)
│   │   ├── SensorTof.*       # Driver VL53L1X (Final)
│   │   └── SensorUltra.*     # Driver HC-SR04 (Prototipo)
│   ├── main.cpp              # Punto de entrada e inicialización
│   ├── Kconfig.projbuild     # Configuración de credenciales
│   └── idf_component.yml     # Dependencias de componentes
├── lib/
│   └── Musica/               # Librería de melodías (buzzer)
├── platformio.ini            # Configuración de entornos
├── partitions.csv            # Tabla de particiones flash
├── sdkconfig.defaults        # Valores por defecto de Kconfig
├── sdkconfig.esp32           # Configuración específica Prototipo
└── sdkconfig.esp32-s3        # Configuración específica Final
```
