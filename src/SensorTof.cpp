#include "SensorTof.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/i2c.h"

static const char* TAG = "SensorToF";

SensorTof::SensorTof(Multiplexor* _mu, const uint8_t* _can, int _maxd):
	mu(_mu),
	maxd(_maxd)
{
	for(int i = 0; i < NUM_TOF; i++){
		can[i] = _can[i];
		tof[i] = nullptr;
	}
}

bool SensorTof::begin(){
	//configuracion necesaria para la libreria
	bool b = true;
	espp::Vl53l::Config config;
    config.device_address = 0x29;

	//funcion de escritura
	config.write = [](uint8_t dev_addr, const uint8_t *data, size_t len) -> bool {
        esp_err_t err = i2c_master_write_to_device(I2C_NUM_0, dev_addr, (uint8_t*)data, len, pdMS_TO_TICKS(10));
    	return (err == ESP_OK);
    };

	//funcion de lectura
	config.read = [](uint8_t dev_addr, const uint8_t *data, size_t len) -> bool {
        esp_err_t err = i2c_master_read_from_device(I2C_NUM_0, dev_addr, (uint8_t*)data, len, pdMS_TO_TICKS(10));
    	return (err == ESP_OK);
    };

	//se aplica para cada SensorTof
	for(int i = 0; i < NUM_TOF; i++){
		//se selecciona canal
		mu->sel(can[i]);
		vTaskDelay(pdMS_TO_TICKS(10));

		//creamos los objetos de los sensores tof
		tof[i] = new espp::Vl53l(config);

		//variable de error
		std::error_code ec;

		//los inicializamos
		if(!tof[i]->start_ranging(ec)){
			ESP_LOGE(TAG, "Fallo al iniciar ToF en canal %d", can[i]);
			b = false;
		}else{
			ESP_LOGI(TAG, "ToF en canal %d iniciado correctamente", can[i]);
		}
	}
	return b;
}

uint16_t SensorTof::dist(int n){
	//verifica si el sensor se creo correctamente
	if(n < 0 || n >= NUM_TOF || tof[n] == nullptr){
		//valor predeterminado de fuera de rango
		return 8190;
	}

	//selecciona el sensor indicado
	mu->sel(can[n]);

	//variable de error
	std::error_code ec;

	uint16_t distancia = tof[n]->get_distance_mm(ec);

	//detecta si hubo algun error
	if (ec) {
        return 8190;
    }

    return distancia;
}

bool SensorTof::verify(int n){
	//se lee la distancia
	uint16_t distancia = dist(n);
	//se verifica si esta en el rango
	if(distancia > 10 && distancia < maxd && distancia < 8000){
		return true;
	}else{
		return false;
	}
}
