#ifndef SENSORLIMITE_H
#define SENSORLIMITE_H

#include "driver/i2c.h"
#include "freertos/task.h"

class SensorLimite{
	private:
		int limCol;//variable de la tolerancia
		bool estado;//variable que verifica el estado de sc_1
		bool estado2;//variable que verifica el estado de sc_2

		//variables de color predeterminado
		int redC;
		int green;
		int blue;

		//se establecen lo valores predetermindos
		uint16_t lcr, lcg, lcb;

		//metodo que selecciona entre sc_1 y sc_2
		void scSel(uint8_t i);

		//metodo de la comunicacion i2c
		void i2c();

		//metodo para leer color
		bool read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
	public:
		//constructor
		SensorLimite(int limCol);

		//metodo que calibra el Sensor
		void calCol();

		//metodos de deteccion de limite
		bool sc_1Verify();
		bool sc_2Verify();

		//inicializara todo
		void begin();
};
#endif
