#ifndef SENSORLIMITE_H
#define SENSORLIMITE_H

#include "driver/i2c.h"
#include "freertos/task.h"
#include "Multiplexor.h"

class SensorLimite{
	private:
		int limCol;//variable de la tolerancia
		bool estado;//variable que verifica el estado de sc_1
		bool estado2;//variable que verifica el estado de sc_2

		//puntero al mutex
		SemaphoreHandle_t* mutex;

		//objeto del Multiplexor
		Multiplexor* mu;

		//variables de color predeterminado
		int redC;
		int green;
		int blue;

		//se establecen lo valores predetermindos
		uint16_t lcr, lcg, lcb;
		//variables de calibracion en el segundo sensor de color
		uint16_t lcr2, lcg2, lcb2;

		//metodo para leer color
		bool read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
		void nvsLeer();

		//valores de los colores leidos
		uint16_t r1;
		uint16_t g1;
		uint16_t b1;
		uint16_t r2;
		uint16_t g2;
		uint16_t b2;
	public:
		//constructor
		SensorLimite(int limCol, Multiplexor* _mu, SemaphoreHandle_t* _mutex);

		//metodo que calibra el Sensor
		void calCol();

		//metodos de deteccion de limite
		bool sc_1Verify();
		bool sc_2Verify();

		//inicializara todo
		void begin();

		//metodo que devuelve las lecturas de los colores
		void colores(uint16_t* rc, uint16_t* gc, uint16_t* bc, uint16_t* red1, uint16_t* green1, uint16_t* blue1, uint16_t* red2, uint16_t* green2, uint16_t* blue2);
};
#endif
