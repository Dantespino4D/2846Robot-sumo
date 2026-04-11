#ifndef PINES_H
#define PINES_H

#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3
    // Pines para ESP32-S3
    #define MUS GPIO_NUM_47
    #define INI GPIO_NUM_2

    // Ultrasonicos
    #define TRIG_1 GPIO_NUM_15
    #define ECHO_1 GPIO_NUM_21
    #define TRIG_2 GPIO_NUM_38
    #define ECHO_2 GPIO_NUM_48

    // Motores
    #define MOT_A1 GPIO_NUM_4
    #define MOT_A2 GPIO_NUM_5
    #define MOT_B1 GPIO_NUM_6
    #define MOT_B2 GPIO_NUM_7

	// Sensores TCRT
	#define TCRT_1 GPIO_NUM_1
	#define TCRT_2 GPIO_NUM_10

    // ToF XSHUT
    #define XSHUT_1 GPIO_NUM_11
    #define XSHUT_2 GPIO_NUM_12
    #define XSHUT_3 GPIO_NUM_13
    #define XSHUT_4 GPIO_NUM_14
    #define XSHUT_5 GPIO_NUM_16
    #define XSHUT_6 GPIO_NUM_17
#else
    // Pines para ESP32 Estándar
    #define MUS GPIO_NUM_4
    #define INI GPIO_NUM_26

    // Ultrasonicos(se comparten con algunos de los pines de XSHUT, ya que nunca se usaran ambos sensores simultaneamente)
    #define TRIG_1 GPIO_NUM_19
    #define ECHO_1 GPIO_NUM_23
    #define TRIG_2 GPIO_NUM_18
    #define ECHO_2 GPIO_NUM_25

    // Motores
    #define MOT_A1 GPIO_NUM_13
    #define MOT_A2 GPIO_NUM_14
    #define MOT_B1 GPIO_NUM_16
    #define MOT_B2 GPIO_NUM_17

	// Sensores TCRT
	#define TCRT_1 GPIO_NUM_34
	#define TCRT_2 GPIO_NUM_35

    // ToF XSHUT
    #define XSHUT_1 GPIO_NUM_19 // Comparte con TRIG_1
    #define XSHUT_2 GPIO_NUM_23 // Comparte con ECHO_1
    #define XSHUT_3 GPIO_NUM_18 // Comparte con TRIG_2
    #define XSHUT_4 GPIO_NUM_25 // Comparte con ECHO_2
    #define XSHUT_5 GPIO_NUM_27
    #define XSHUT_6 GPIO_NUM_5
#endif

#endif
