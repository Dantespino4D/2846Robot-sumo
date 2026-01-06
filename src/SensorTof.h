#ifndef SENSORTOF_H
#define SENSORTOF_H

#include "vl53l.hpp"
#include "Multiplexor.h"
#include <vector>

//numero de sensores ToF
#define NUM_TOF 6

class SensorTof{
	private:
		//el multiplexor
		Multiplexor* mu;
		//puntero al mutex
		SemaphoreHandle_t* mutex;
		//objetos toF de la libreria
		espp::Vl53l* tof[NUM_TOF];
		//canales
		uint8_t can[NUM_TOF];
		//distancia maxima
		int maxd;
		//variables de las lecturas de los sensores
		uint16_t dis[NUM_TOF];
		//lee determinado sensor ToF
		uint16_t dist(int n);
		//metodo que lee si hay una distancia maxima en nvs
		void nvsLeer();
	public:
		//constructor
		SensorTof(Multiplexor* _mu, SemaphoreHandle_t* _mutex, const uint8_t* _can, int _maxd);
		//inicializa los sensores ToF
		bool begin();
		//verifica si detecto un enemigo
		bool verify(int n);
		//metodo que envia las medidas de cada sensor a la telemetria
		void distancias(uint16_t* t1, uint16_t* t2, uint16_t* t3, uint16_t* t4, uint16_t* t5, uint16_t* t6 );
};
#endif
