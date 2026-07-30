#ifndef PINES_H
#define PINES_H

#include "driver/gpio.h"

    // inicio
    #define INI GPIO_NUM_35

	//interrupcion stm32
	#define STM32_INT GPIO_NUM_5

	//musica
    #define MUS GPIO_NUM_3

	//spi
	#define SPI_MOSI GPIO_NUM_42
	#define SPI_MISO GPIO_NUM_41
	#define SPI_CLK GPIO_NUM_40
	#define SPI_CS GPIO_NUM_39

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
#endif
