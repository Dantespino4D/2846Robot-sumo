#include "Multiplexor.h"       // Tu propia cabecera
#include "sdkconfig.h"

// Librerías del sistema ESP-IDF
#include "esp_log.h"           // Para usar ESP_LOGE, ESP_LOGI (Debugging)
#include "freertos/FreeRTOS.h" // Necesario para definiciones base de RTOS
#include "freertos/task.h"     // Necesario para vTaskDelay y pdMS_TO_TICKS

// Librerías de tu proyecto
#include "rgb.h"               // Para llamar a rgb(0, 1023) si falla el inicio
							   //
//pines ic2
#ifdef CONFIG_IDF_TARGET_ESP32S3
    // Pines estandar I2C para ESP32-S3 (DevKitC-1)
    #define I2C_MASTER_SCL_IO GPIO_NUM_9
    #define I2C_MASTER_SDA_IO GPIO_NUM_8
#else
    #define I2C_MASTER_SCL_IO GPIO_NUM_22
    #define I2C_MASTER_SDA_IO GPIO_NUM_21
#endif
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

// direccion del multiplexor y sensor
#define TCAADDR 0x70

//tag
static const char* TAG = "Multiplexor";

void Multiplexor::begin(){
	//configuracion del i2c
	i2c_config_t conf = {};
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_MASTER_SDA_IO;
	conf.scl_io_num = I2C_MASTER_SCL_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	i2c_param_config(I2C_MASTER_NUM, &conf);

	esp_err_t err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	if (err != ESP_OK){
		ESP_LOGE(TAG, "fallo al instalar i2c %s", esp_err_to_name(err));
		rgb(0, 1023);
	}
}


void Multiplexor::sel(uint8_t i){
	if(i > 7){
	return;
	}
	//selecciona el sensor de color a utilizar
    uint8_t data = 1 << i;
	esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, TCAADDR, &data, 1, pdMS_TO_TICKS(20));
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al seleccionar canal del MUX I2C: %s", esp_err_to_name(err));
		rgb(0, 1023);
    }
}

 i2c_port_t Multiplexor::port(){
	return I2C_MASTER_NUM;
}

