#ifndef SENSORLIMITE_H
#define SENSORLIMITE_H

#include <Arduino.h>
#include <Adafruit_TCS34725.h>
#include <Wire.h>

class SensorLimite{
	private:
		int limCol;//variable de la tolerancia
		bool estado;//variable que verifica el estado de sc_1
		bool estado2;//variable que verifica el estado de sc_2

		//variables de color predeterminado
		int redC;
		int green;
		int blue;

		//se declaran sc_1 y sc_2
		Adafruit_TCS34725 sc_1;
		Adafruit_TCS34725 sc_2;
		//se establecen lo valores predetermindos
		uint16_t lcr, lcg, lcb;

		//metodo que selecciona entre sc_1 y sc_2
		void scSel(uint8_t i);
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
