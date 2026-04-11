#ifndef SENSORTOF_H
#define SENSORTOF_H

#include "vl53l.hpp"
#include "SensorRival.h"
#include "../core/GestorI2C.h"
#include "driver/gpio.h"
#include <vector>

//numero de sensores ToF
#define NUM_TOF 6

class SensorTof : public SensorRival {
	private:
		//gestor I2C
		GestorI2C& i2c;
		//pines xshut
		gpio_num_t xshut[NUM_TOF];
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
		//verifica si detecto un enemigo en determinado sensor
		bool verify(int n);
	public:
		//constructor
		SensorTof(GestorI2C& _i2c, const uint8_t* _can, int _maxd);
		//destructor
		~SensorTof();
		//inicializa los sensores ToF
		bool begin() override;
		//verifica cada uno de los sensores ToF para enviar las notificaciones correspondientes
		void procesar() override;
		//metodo que envia las medidas de cada sensor a la telemetria
		void getDistancias(uint16_t* buffer) override;
};
#endif
