#include "SensorTof.h"
#include "../core/Nvs.h"
#include "../actuadores/rgb.h"
#include "../configuracion/eventos.h"
#include "../configuracion/pines.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/i2c.h"

static const char* TAG = "SensorToF";

SensorTof::SensorTof(GestorI2C& _i2c, const uint8_t* _can, int _maxd):
	i2c(_i2c),
	xshut{XSHUT_1, XSHUT_2, XSHUT_3, XSHUT_4, XSHUT_5, XSHUT_6},
	maxd(_maxd)
{
	for(int i = 0; i < NUM_TOF; i++){
		can[i] = _can[i];
		tof[i] = nullptr;
		dis[i] = 0;
	}
}

SensorTof::~SensorTof() {
	for(int i = 0; i < NUM_TOF; i++){
		if (tof[i] != nullptr) {
			delete tof[i];
			tof[i] = nullptr;
		}
	}
}

bool SensorTof::begin(){
	//carga los datos del nvs
	nvsLeer();

	//configuracion necesaria para la libreria
	bool b = true;

	//configuracion de lso pines xshut
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 0;
	for(int i = 0; i < NUM_TOF; i++){
		io_conf.pin_bit_mask |= (1ULL << xshut[i]);
	}
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);

	//se apagan todos los xshut
	for(int i = 0; i < NUM_TOF; i++){
		gpio_set_level(xshut[i], 0);
	}

	//pequeña espera
	vTaskDelay(pdMS_TO_TICKS(10));

	//se aplica para cada SensorTof
	for(int i = 0; i < NUM_TOF; i++){
		//ensendemos el xshut
		gpio_set_level(xshut[i], 1);
		vTaskDelay(pdMS_TO_TICKS(10));

		uint8_t nueva_dir = 0x30 + i;

		//se cambia la direccion del sensor fisicamente enviando el comando I2C
		uint8_t payload[2] = {0x8A, (uint8_t)(nueva_dir & 0x7F)};
		esp_err_t err_dir = i2c_master_write_to_device(i2c.port(), 0x29, payload, 2, pdMS_TO_TICKS(10));

		if(err_dir == ESP_OK){
			ESP_LOGI(TAG, "ToF en canal %d configurado con direccion 0x%02X", can[i], nueva_dir);
		} else {
			ESP_LOGE(TAG, "Fallo al configurar direccion del ToF en canal %d", can[i]);
			b = false;
		}

		if (tof[i] != nullptr) {
    	    delete tof[i];
    	}

		espp::Vl53l::Config config;
		// Usamos directamente la nueva direccion
		config.device_address = nueva_dir;

		//funcion de escritura
		config.write = [this](uint8_t dev_addr, const uint8_t *data, size_t len) -> bool {
			esp_err_t err = i2c_master_write_to_device(i2c.port(), dev_addr, (uint8_t*)data, len, pdMS_TO_TICKS(10));
			if(err != ESP_OK){
				rgb(0, 1023);
			}
			return (err == ESP_OK);
		};

		//funcion de lectura
		config.read = [this](uint8_t dev_addr, uint8_t *data, size_t len) -> bool {
			esp_err_t err = i2c_master_read_from_device(i2c.port(), dev_addr, data, len, pdMS_TO_TICKS(10));
			if(err != ESP_OK){
				rgb(0, 1023);
			}
			return (err == ESP_OK);
		};

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

	//variable de error
	std::error_code ec;

	if(!tof[n]->is_data_ready(ec)){
		//detecta si hubo algun error
		if(ec){
			return 8190;
		}
		//si no hay datos listos, retorna el valor anterior
		return dis[n];
	}
	//lee la distancia
	dis[n] = tof[n]->get_distance_mm(ec);

	//limpia la interrupcion de datos listos
	tof[n]->clear_interrupt(ec);

	//detecta si hubo algun error
	if (ec) {
        return 8190;
    }
    return dis[n];
}

//metodo que verifica cada sensor
bool SensorTof::verify(int n){
	//variable que registrara el resultado
	bool res = false;
	//se lee la distancia
	uint16_t distancia = dist(n);
	//se verifica si esta en el rango
	if(distancia > 10 && distancia < maxd && distancia < 8000){
		res = true;
	}
	return res;
}

//metodo que lee todos los sensores
void SensorTof::procesar(){
	for(uint8_t i = 0; i < NUM_TOF; i++){
		//se lee la distancia
    	uint16_t distancia = dist(i);
		//verifica que la distancia sea valida
		if(distancia > 10 && distancia < maxd && distancia < 8000){
			//envia el bit del tof correspondiente
			xEventGroupSetBits(eventos, TOF_BITS[i]);
		} else {
			//borra el bit del tof correspondiente
			xEventGroupClearBits(eventos, TOF_BITS[i]);
		}
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
