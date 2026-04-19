#ifndef SENSORTOF_H
#define SENSORTOF_H

#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "vl53l.hpp"
#include "SensorRival.h"
#include "../core/GestorI2C.h"
#include "driver/gpio.h"
#include <vector>
#include "esp_attr.h"

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
	public:
		//struct de los datos leidos
		struct TofData {
			uint16_t distancia;
			uint16_t estado;
			uint16_t señal;
			uint16_t ambiente;
		};

	private:
		//gestor I2C
		GestorI2C& i2c;
		//el controlador de la tarea
		static TaskHandle_t tarea;
		//instancia de la clase para la interrupcion
		static SensorTof* instancia;
		//pines xshut
		gpio_num_t xshut[NUM_TOF];
		//pines de interrupcion
		gpio_num_t intp[NUM_TOF];
		//objetos toF de la libreria
		MiVl53l* tof[NUM_TOF];
		//direcciones I2C
		uint8_t dir[NUM_TOF];
		//arreglo donde se marca cual sensor tiene un dato listo
		uint8_t listo[NUM_TOF];
		//distancia maxima
		int maxd;
		//variables de las lecturas de los sensores
		TofData data[NUM_TOF];
		//metodo que lee si hay una distancia maxima en nvs
		void nvsLeer();

	public:
		//constructor
		SensorTof(GestorI2C& _i2c, const uint8_t* _dir, int _maxd);
		//destructor
		~SensorTof();
		//inicializa los sensores ToF
		bool begin() override;
		//leera los valores de los sensores
		TofData dist(uint8_t i2c_dir);
		//atiende las interrupciones
		static void IRAM_ATTR tofIntr(void* arg);
		//tarea que procesara los datos
		static void tareaTof(void* pvParameters);
		//metodo que envia las medidas de cada sensor a la telemetria
		void getDistancias(uint16_t* buffer) override;
		void procesar() override {}
};

#endif // CONFIG_IDF_TARGET_ESP32S3
#endif // SENSORTOF_H
