#include "SensorTof.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "../core/Nvs.h"
#include "../actuadores/rgb.h"
#include "../configuracion/eventos.h"
#include "../configuracion/pines.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
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
		dis[i] = 8190;
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

	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 0;
	for(int i = 0; i < NUM_TOF; i++){
		io_conf.pin_bit_mask |= (1ULL << xshut[i]);
	}
	gpio_config(&io_conf);

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
			return i2c_master_write_to_device(i2c.port(), dev_addr, data, len, pdMS_TO_TICKS(10)) == ESP_OK;
		};
		config.read = [this](uint8_t dev_addr, uint8_t *data, size_t len) -> bool {
			return i2c_master_read_from_device(i2c.port(), dev_addr, data, len, pdMS_TO_TICKS(10)) == ESP_OK;
		};

		tof[i] = new MiVl53l(config);

		std::error_code ec;
		uint8_t nueva_dir = can[i];
		uint8_t payload[3] = {0x00, 0x01, (uint8_t)(nueva_dir & 0x7F)};
		if (i2c_master_write_to_device(i2c.port(), 0x29, payload, 3, pdMS_TO_TICKS(10)) == ESP_OK) {
			tof[i]->set_sensor_address(nueva_dir);
			ESP_LOGI(TAG, "ToF #%d listo en 0x%02X", i, nueva_dir);
		} else {
			ESP_LOGE(TAG, "Fallo remapeo ToF #%d", i);
			b = false;
		}

		tof[i]->set_inter_measurement_period_ms(0, ec);
		tof[i]->set_timing_budget_ms(20, ec);

		if(!tof[i]->start_ranging(ec)){
			ESP_LOGE(TAG, "Error iniciando ranging en ToF #%d", i);
			b = false;
		}
	}
	return b;
}

void SensorTof::dist(int n){

}

void SensorTof::procesar(){
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
        buffer[i+2] = dis[i];
    }
}

#endif // CONFIG_IDF_TARGET_ESP32S3
