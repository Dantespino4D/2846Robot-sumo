#ifndef PINES_H
#define PINES_H

#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3
    // Pines para ESP32-S3
    #define MUS GPIO_NUM_3
    #define INI GPIO_NUM_35

    // Lectura de ADC
    #define BAT GPIO_NUM_2

    // Motores
    #define MOT_A1 GPIO_NUM_39
    #define MOT_A2 GPIO_NUM_40
    #define MOT_B1 GPIO_NUM_41
    #define MOT_B2 GPIO_NUM_42

	// Sensores TCRT
	#define TCRT_1 GPIO_NUM_5
	#define TCRT_2 GPIO_NUM_10
	#define TCRT_3 GPIO_NUM_6
	#define TCRT_4 GPIO_NUM_21

    // Hardware Faults para MCPWM (Señales combinadas)
    #define FAULT_A GPIO_NUM_1
    #define FAULT_B GPIO_NUM_7

    // I2C
    #define SDA GPIO_NUM_8
    #define SCL GPIO_NUM_9

    // ToF XSHUT
    #define XSHUT_1 GPIO_NUM_11
    #define XSHUT_2 GPIO_NUM_12
    #define XSHUT_3 GPIO_NUM_13
    #define XSHUT_4 GPIO_NUM_14
    #define XSHUT_5 GPIO_NUM_16
    #define XSHUT_6 GPIO_NUM_17

    // ToF Interrupciones
    #define INT_1 GPIO_NUM_15
    #define INT_2 GPIO_NUM_4
    #define INT_3 GPIO_NUM_38
    #define INT_4 GPIO_NUM_48
    #define INT_5 GPIO_NUM_18
    #define INT_6 GPIO_NUM_47

    // LED RGB
    #define RGB_PIN_A GPIO_NUM_43
    #define RGB_PIN_B GPIO_NUM_44

    // Botones Auxiliares
    #define VIC GPIO_NUM_36
    #define DER GPIO_NUM_37
#else
    // Pines para ESP32 Estándar (PROTOTIPO)
    #define MUS GPIO_NUM_4
    #define INI GPIO_NUM_26

    // I2C
    #define SDA GPIO_NUM_21
    #define SCL GPIO_NUM_22

    // Ultrasonicos
    #define TRIG_1 GPIO_NUM_19
    #define ECHO_1 GPIO_NUM_23
    #define TRIG_2 GPIO_NUM_18
    #define ECHO_2 GPIO_NUM_25

    // Motores
    #define MOT_A1 GPIO_NUM_13
    #define MOT_A2 GPIO_NUM_14
    #define MOT_B1 GPIO_NUM_16
    #define MOT_B2 GPIO_NUM_17

	// LED RGB
    #define RGB_PIN_A GPIO_NUM_32
    #define RGB_PIN_B GPIO_NUM_33
#endif

#endif
