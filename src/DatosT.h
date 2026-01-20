#ifndef DATOS_H
#define DATOS_H

#include <cstdint>

struct Datos{
	//variabel del estado actual
	int estado;
	int estrategia;
	int inicio;

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

	//variables de la fiablilidad de los sensores ToF
	uint16_t fToF1;
	uint16_t fToF2;
	uint16_t fToF3;
	uint16_t fToF4;
	uint16_t fToF5;
	uint16_t fToF6;

	//variables de la calibracion de los sensores de color
	uint16_t cR1;
	uint16_t cG1;
	uint16_t cB1;
	uint16_t cC1;
	uint16_t cR2;
	uint16_t cG2;
	uint16_t cB2;
	uint16_t cC2;


	//variables del primero sensor de color
	uint16_t scR1;
	uint16_t scG1;
	uint16_t scB1;
	uint16_t scC1;

	//variables del segundo sensor de color
	uint16_t scR2;
	uint16_t scG2;
	uint16_t scB2;
	uint16_t scC2;

	//variable de la pila
	float pila;

	//variables de los motores
	int16_t pwm1;
	int16_t pwm2;
	int stall;

	//variables del sistema
	uint32_t tiempo;
	uint32_t heap;
	float temperatura;
	int wifi;
	int ciclo;
	int prototipo;
};

#endif
