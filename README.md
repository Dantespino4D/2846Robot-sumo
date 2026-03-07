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

- **`MaquinaEstados`** — Núcleo táctico del robot. Soporta múltiples estrategias seleccionables desde NVS (prototipo, E1, E2). Implementa un avanzado sistema de **memoria a corto plazo (Zero-Order Hold)** de 10 estados para evitar "tartamudeos" por el parpadeo o multiplexado de los sensores, y un sistema de **memoria a largo plazo** para la persecución ciega y predictiva del rival.
- **`ControlMotores`** — Abstracción para el control PWM de los 4 motores DC. Define comandos estratégicos de alto nivel: direcciones, ataques directos, giros pronunciados y velocidad máxima.
- **`SensorLimite`** — Lectura en hilo secundario de los sensores de color TCS34725 para evadir el borde blanco del dohyo.
- **`SensorRival`** — Interfaz abstracta diseñada para facilitar la migración de los ultrasónicos HC-SR04 a los sensores ToF VL53L0X sin alterar la lógica superior.
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

## Fortalezas del Diseño

* **Abstracción Orientada a Hardware Evolutivo:** El uso de herencia y polimorfismo en `SensorRival` facilita la transición planificada a los sensores ToF VL53L0X.
* **Diseño Bidireccional:** Aporta una ventaja táctica inmensa, ya que la máquina de estados puede simplemente invertir motores para atacar a un rival trasero sin consumir tiempo valioso en girar.
* **Arquitectura FreeRTOS:** La segregación de la lógica del robot, la lectura del ADC/I2C de sensores de color y la interrupción de motores en distintas tareas priorizadas asegura tiempos de respuesta de milisegundos en combate.
* **Potencia Desacoplada:** El uso de 4 drivers DRV8871 permite aprovechar la máxima corriente de pico por cada rueda de manera individual, previniendo cuellos de botella térmicos frente al empuje extremo.
* **Sistema Anti-Jitter (Zero-Order Hold):** La máquina de estados procesa las lecturas multiplexadas de los sensores a través de una matriz de memorias a corto plazo. Esto permite que el robot mantenga una fluidez perfecta de ataque y previene tirones en los motores si se pierde una lectura de sensor por milisegundos.

---

## Estructura de Archivos


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
