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
		//objetos toF de la libreria
		espp::Vl53l* tof[NUM_TOF];
		//canales
		uint8_t can[NUM_TOF];
		//distancia maxima
		int maxd;
	public:
		//constructor
		SensorTof(Multiplexor* _mu, const uint8_t* _can, int _maxd);
		//inicializa los sensores ToF
		bool begin();
		//lee determinado sensor ToF
		uint16_t dist(int n);
		//verifica si detecto un enemigo
		bool verify(int n);
};
#endif
