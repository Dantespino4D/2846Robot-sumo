#ifndef SENSORTOF_H
#define SENSORTOF_H

#include "vl53l.hpp"
#include "SensorRival.h"
#include "../actuadores/Multiplexor.h"
#include <vector>

//numero de sensores ToF
#define NUM_TOF 6

class SensorTof : public SensorRival {
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
		//contador que define de quien es turno
		int tur;
		//lee determinado sensor ToF
		uint16_t dist(int n);
		//metodo que lee si hay una distancia maxima en nvs
		void nvsLeer();
		//verifica si detecto un enemigo en determinado sensor
		bool verify(int n);
	public:
		//constructor
		SensorTof(Multiplexor* _mu, SemaphoreHandle_t* _mutex, const uint8_t* _can, int _maxd);
		//inicializa los sensores ToF
		bool begin() override;
		//verifica cada uno de los sensores ToF para enviar las notificaciones correspondientes
		void procesar() override;
		//metodo que envia las medidas de cada sensor a la telemetria
		void getDistancias(uint16_t* buffer) override;
};
#endif
