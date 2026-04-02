#include "SensorTof.h"
#include "../core/Nvs.h"
#include "../actuadores/rgb.h"
#include "../configuracion/eventos.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "driver/i2c.h"

static const char* TAG = "SensorToF";

SensorTof::SensorTof(Multiplexor* _mu, SemaphoreHandle_t* _mutex, const uint8_t* _can, int _maxd):
	mu(_mu),
	mutex(_mutex),
	maxd(_maxd),
	tur(0)
{
	for(int i = 0; i < NUM_TOF; i++){
		can[i] = _can[i];
		tof[i] = nullptr;
		dis[i] = 0;
	}
}

bool SensorTof::begin(){
	//carga los datos del nvs
	nvsLeer();

	//configuracion necesaria para la libreria
	bool b = true;
	espp::Vl53l::Config config;
    config.device_address = 0x29;

	//funcion de escritura
	config.write = [](uint8_t dev_addr, const uint8_t *data, size_t len) -> bool {
        esp_err_t err = i2c_master_write_to_device(I2C_NUM_0, dev_addr, (uint8_t*)data, len, pdMS_TO_TICKS(10));
		if(err != ESP_OK){
			rgb(0, 1023);
		}
    	return (err == ESP_OK);
    };

	//funcion de lectura
	config.read = [](uint8_t dev_addr, const uint8_t *data, size_t len) -> bool {
        esp_err_t err = i2c_master_read_from_device(I2C_NUM_0, dev_addr, (uint8_t*)data, len, pdMS_TO_TICKS(10));
		if(err != ESP_OK){
			rgb(0, 1023);
		}
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

        tof[i]->set_inter_measurement_period_ms(0, ec);

        // Configurar en modo High Speed (20ms timing budget)
        tof[i]->set_timing_budget_ms(20, ec);

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

	dis[n] = tof[n]->get_distance_mm(ec);

	//detecta si hubo algun error
	if (ec) {
		mu->error();
        return 8190;
    }
	mu->reset();
    return dis[n];
}

//metodo que verifica cada sensor
bool SensorTof::verify(int n){
	//variable que registrara el resultado
	bool res = false;
	if(xSemaphoreTake(*mutex, pdMS_TO_TICKS(50)) == pdTRUE){
		//se lee la distancia
		uint16_t distancia = dist(n);
		//se verifica si esta en el rango
		if(distancia > 10 && distancia < maxd && distancia < 8000){
			res = true;
		}
		xSemaphoreGive(*mutex);
	}else{
		rgb(0, 1023);
	}
	return res;
}

//metodo que lee todos los sensores
void SensorTof::procesar(){
    if(xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE){
		//se lee la distancia
	    uint16_t distancia = dist(tur);
		if(distancia > 10 && distancia < maxd && distancia < 8000){
			xEventGroupSetBits(eventos, TOF_BITS[tur]);
		} else {
			xEventGroupClearBits(eventos, TOF_BITS[tur]);
		}
		tur++;
        xSemaphoreGive(*mutex);
    }
	if(tur >= NUM_TOF){
		tur = 0;
	}
}

//metodo que lee la Nvs
void SensorTof::nvsLeer(){
	//se crea el objeto del nvs
	Nvs nvs("sensores");
	//se extrae el valor de la distancia maxima
	maxd = nvs.leer("dist_max", maxd);
	//se convierte de cm a mm
	maxd = maxd * 10;
}

//metodo que envia las medidas de cada sensor a la telemetria
void SensorTof::getDistancias(uint16_t* buffer){
    // Posiciones 0 y 1 reservadas para Ultrasonido
    buffer[0] = 0;
    buffer[1] = 0;
    // Copiamos los 6 sensores ToF a las posiciones 2-7
    for(int i=0; i<NUM_TOF; i++){
        buffer[i+2] = dis[i];
    }
}
