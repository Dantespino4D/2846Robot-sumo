#ifndef DATOS_H
#define DATOS_H

#include <cstdint>

struct Datos{
	//variabel del estado actual
	int estado;

	//variables de los sensores ultrasonicos
	uint16_t ojos1;
	uint16_t ojos2;

	//variable de los sensores ToF
	uint16_t ToF1;
	uint16_t ToF2;
	uint16_t ToF3;
	uint16_t ToF4;
	uint16_t ToF5;
	uint16_t ToF6;

	//variables de la calibracion de los sensores de color
	uint16_t scR;
	uint16_t scG;
	uint16_t scB;

	//variables del primero sensor de color
	uint16_t scR1;
	uint16_t scG1;
	uint16_t scB1;

	//variables del segundo sensor de color
	uint16_t scR2;
	uint16_t scG2;
	uint16_t scB2;

	//variable de la pila
	float pila;

	//variables de los motores
	int16_t pwm1;
	int16_t pwm2;

	//variables del sistema
	uint32_t LoopT;
	uint32_t heap;
};

#endif
