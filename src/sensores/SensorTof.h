#ifndef SENSORTOF_H
#define SENSORTOF_H

#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "vl53l.hpp"
#include "SensorRival.h"
#include "../core/GestorI2C.h"
#include "driver/gpio.h"
#include <vector>

//numero de sensores ToF
#define NUM_TOF 6

class MiVl53l : public espp::Vl53l {
	public:
		using espp::Vl53l::Vl53l;
        // Metodo para actualizar la direccion I2C del sensor
        void set_sensor_address(uint8_t addr) {
            this->espp::BasePeripheral<espp::vl53l_register_t, true>::set_address(addr);
        }
};

class SensorTof : public SensorRival {
	private:
		//gestor I2C
		GestorI2C& i2c;
		//pines xshut
		gpio_num_t xshut[NUM_TOF];
		//objetos toF de la libreria
		MiVl53l* tof[NUM_TOF];
		//canales
		uint8_t can[NUM_TOF];
		//distancia maxima
		int maxd;
		//variables de las lecturas de los sensores
		uint16_t dis[NUM_TOF];
		//metodo que lee si hay una distancia maxima en nvs
		void nvsLeer();

	public:
		//constructor
		SensorTof(GestorI2C& _i2c, const uint8_t* _can, int _maxd);
		//destructor
		~SensorTof();
		//inicializa los sensores ToF
		bool begin() override;
		//leera los valores de los sensores
		void dist(int n);
		//verifica cada uno de los sensores ToF para enviar las notificaciones correspondientes
		void procesar() override;
		//metodo que envia las medidas de cada sensor a la telemetria
		void getDistancias(uint16_t* buffer) override;
};

#endif // CONFIG_IDF_TARGET_ESP32S3
#endif // SENSORTOF_H
