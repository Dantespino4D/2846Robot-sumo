#include "SensorTof.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "Nvs.h"
#include "rgb.h"
#include "eventos.h"
#include "pines.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_heap_caps.h"
#include "portmacro.h"
#include <cstddef>
#include <system_error>
#include "esp_task_wdt.h"

static const char* TAG = "SensorToF";

SensorTof* SensorTof::instancia = nullptr;

SensorTof::SensorTof(GestorI2C& _i2c, const uint8_t* _dir, int _maxd):
	i2c(_i2c),
	xshut{XSHUT_1, XSHUT_2, XSHUT_3, XSHUT_4, XSHUT_5, XSHUT_6},
	intp{INT_1, INT_2, INT_3, INT_4, INT_5, INT_6},
	listo{0},
	maxd(_maxd)
{
	for(int i = 0; i < NUM_TOF; i++){
		dir[i] = _dir[i];
		tof[i] = nullptr;
		listo[i] = 0;
		data[i].distancia = 8190;
		data[i].estado = 0;
		data[i].señal = 0;
		data[i].ambiente = 0;
		tx = (uint8_t*)heap_caps_malloc(32, MALLOC_CAP_DMA | MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
		rx = (uint8_t*)heap_caps_malloc(32, MALLOC_CAP_DMA | MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
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
	nvsLeer();
	bool b = true;
	instancia = this;

	//configuracion de los pines xshut
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 0;
	for(int i = 0; i < NUM_TOF; i++){
		io_conf.pin_bit_mask |= (1ULL << xshut[i]);
	}
	gpio_config(&io_conf);

	//configuracion de los pines de interrupcion
	gpio_config_t io_conf_intp = {};
	io_conf_intp.intr_type = GPIO_INTR_NEGEDGE;
	io_conf_intp.mode = GPIO_MODE_INPUT;
	io_conf_intp.pin_bit_mask = 0;
	for(int i = 0; i < NUM_TOF; i++){
		io_conf_intp.pin_bit_mask |= (1ULL << intp[i]);
	}
	gpio_config(&io_conf_intp);

	//instalar las interrupciones si no estan instaladas
	esp_err_t isr_err = gpio_install_isr_service(0);
	if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) {
		ESP_LOGE(TAG, "Error instalando ISR service ToF: %s", esp_err_to_name(isr_err));
	}

	//añadir las interrupciones
	for(int i = 0; i < NUM_TOF; i++){
		gpio_isr_handler_add(intp[i], &SensorTof::tofIntr, (void*)i);
	}

	//apagado general de los xsgut
	for(int i = 0; i < NUM_TOF; i++){
		gpio_set_level(xshut[i], 0);
	}
	vTaskDelay(pdMS_TO_TICKS(10));

	for(int i = 0; i < NUM_TOF; i++){
		gpio_set_level(xshut[i], 1);
		vTaskDelay(pdMS_TO_TICKS(10));

		espp::Vl53l::Config config;
		config.device_address = 0x29;

		config.write = [this](uint8_t dev_addr, const uint8_t *data, size_t len) -> bool {
            i2c_master_dev_handle_t dev = i2c.get_device(dev_addr);
            if (!dev) return false;
			return i2c_master_transmit(dev, data, len, pdMS_TO_TICKS(10)) == ESP_OK;
		};
		config.read = [this](uint8_t dev_addr, uint8_t *data, size_t len) -> bool {
            i2c_master_dev_handle_t dev = i2c.get_device(dev_addr);
            if (!dev) return false;
			return i2c_master_receive(dev, data, len, pdMS_TO_TICKS(10)) == ESP_OK;
		};

		tof[i] = new MiVl53l(config);

		std::error_code ec;
		uint8_t nueva_dir = dir[i];
		uint8_t payload[3] = {0x00, 0x01, (uint8_t)(nueva_dir & 0x7F)};

        i2c_master_dev_handle_t dev29 = i2c.get_device(0x29);
		if (dev29 && i2c_master_transmit(dev29, payload, 3, pdMS_TO_TICKS(10)) == ESP_OK) {
			tof[i]->set_sensor_address(nueva_dir);
			ESP_LOGI(TAG, "ToF #%d listo en 0x%02X", i, nueva_dir);
		} else {
			ESP_LOGE(TAG, "Fallo remapeo ToF #%d", i);
			b = false;
		}

		tof[i]->set_inter_measurement_period_ms(0, ec);
		tof[i]->set_timing_budget_ms(33, ec);

		// Configurar ROI: 16x4 en la parte superior (centro 231)
		tof[i]->set_roi(16, 4, 231);

		if(!tof[i]->start_ranging(ec)){
			ESP_LOGE(TAG, "Error iniciando ranging en ToF #%d", i);
			b = false;
		}
	}
	//creacion de la tarea
	tarea = xTaskCreateStaticPinnedToCore(SensorTof::tareaTof, "tareaTof", sizeof(stackTof), this, 10, stackTof, &tcbTof, 0);
	return b;
}

void SensorTof::reasignar(){
	for(int i = 0; i < NUM_TOF; i++){
		gpio_set_level(xshut[i], 0);
	}
	vTaskDelay(pdMS_TO_TICKS(10));
	for(int i = 0; i < NUM_TOF; i++){
		gpio_set_level(xshut[i], 1);
		vTaskDelay(pdMS_TO_TICKS(5));
		uint8_t payload[3] = {0x00, 0x01, (uint8_t)(dir[i] & 0x7F)};
        i2c_master_dev_handle_t dev29 = i2c.get_device(0x29);
		if (dev29) {
            i2c_master_transmit(dev29, payload, 3, pdMS_TO_TICKS(10));
        }
		std::error_code error;

		if(tof[i] != nullptr){
			tof[i]->start_ranging(error);
		}
	}
	ESP_LOGI(TAG, "Reasignacion de direcciones ToF completada");
}

SensorTof::TofData SensorTof::dist(uint8_t i2c_dir){
	//preparamos los buffers
	uint8_t inicio[2] = {0x00, 0x89};
	uint8_t datos[15];

    i2c_master_dev_handle_t dev = i2c.get_device(i2c_dir);

	//iniciamos lectua
	esp_err_t fallo = dev ? i2c_master_transmit_receive(dev, inicio, 2, datos, 15, pdMS_TO_TICKS(2)) : ESP_FAIL;
	
    //guardamos los datos en el struct
	TofData res;
	res.distancia = (datos[13] << 8) | datos[14];
	res.estado = datos[0] & 0x1F;
	res.señal = (datos[5] << 8) | datos[6];
	res.ambiente = (datos[7] << 8) | datos[8];

	//limpiamos el registro de interrupcion
	uint8_t clear[3] = {0x00, 0x86, 0x01};
	esp_err_t limpiar = dev ? i2c_master_transmit(dev, clear, 3, pdMS_TO_TICKS(2)) : ESP_FAIL;

	//evaluaos errores
	if(fallo != ESP_OK || limpiar != ESP_OK){
		//manejamos el error
		i2c.error();
		res.distancia = 8190;
		res.estado = 255;
	}else{
		i2c.reset();
	}
	//retornamos el resultado
	return res;
}

void IRAM_ATTR SensorTof::tofIntr(void* arg){
	//recibe el puntero del sensor que genero la interrupcion
	uint8_t tof =(uint8_t) (uint32_t) arg;
	BaseType_t cambioC = pdFALSE;

	if (instancia == nullptr || instancia->tarea == NULL) {
		return;
	}

	//se marca cual fue el sensor que dio la interrupcion
	instancia->listo[tof] = true;

	vTaskNotifyGiveFromISR(instancia->tarea, &cambioC);

	//cambia el estado del bit correspondiente al sensor que genero la interrupcion
	if(cambioC){
		portYIELD_FROM_ISR();
	}
}

void SensorTof::tareaTof(void* pvParameters){
	//suscribir la tarea al watchdog
	esp_err_t err = esp_task_wdt_add(NULL);
	if(err != ESP_OK){
		ESP_LOGE(TAG, "Error al suscribir al watchdog: %s", esp_err_to_name(err));
	}


	SensorTof* sensor = (SensorTof*)pvParameters;
	while(true){
		esp_task_wdt_reset();
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
		//verificamos que el i2c este funcionando
		if(!sensor->i2c.verify()){
			vTaskDelay(pdMS_TO_TICKS(5));
			if(sensor->dist(sensor->dir[0]).estado == 255){
				ESP_LOGE(TAG, "direcciones perdida");
				sensor->reasignar();
			}
			continue;
		}
		//procesamos los sensores que tengan un dato listo
		for(int i = 0; i < NUM_TOF; i++){
			//verificamos si el sensor tiene un dato listo
			if(sensor->listo[i]){
				//se recolectan los datos
				sensor->listo[i] = 0;
				TofData res = sensor->dist(sensor->dir[i]);
				sensor->data[i] = res;

				//validaciones y eventos
				if(res.estado == 0 && res.distancia < sensor->maxd){
					//evento de deteccion
					xEventGroupSetBits(eventos, TOF_BITS[i]);
				}else{
					//evento de no deteccion
					xEventGroupClearBits(eventos, TOF_BITS[i]);
					//valor de error o fuera de rango
					sensor->data[i].distancia = 8190;
				}
			}
		}
	}
	vTaskDelete(NULL);
}

void SensorTof::nvsLeer(){
	Nvs nvs("sensores");
	maxd = nvs.leer("dist_max", maxd);
	maxd = maxd * 10;
}

void SensorTof::getDistancias(uint16_t* buffer){
    buffer[0] = 0;
    buffer[1] = 0;
    for(int i=0; i<NUM_TOF; i++){
        buffer[i] = data[i].distancia;
		buffer[i+6] = data[i].estado;
		buffer[i+12] = data[i].señal;
		buffer[i+18] = data[i].ambiente;
    }
}

#endif // CONFIG_IDF_TARGET_ESP32S3
