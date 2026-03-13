#include "Multiplexor.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "rgb.h"
//pines ic2
#ifdef CONFIG_IDF_TARGET_ESP32S3
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
	//se establecen los errores en 0
	err = 0;
	//configuracion del i2c
	conf = {};
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
		error();
		return;
    }
	reset();
}

 i2c_port_t Multiplexor::port(){
	return I2C_MASTER_NUM;
}

//metodo que suma errores
void Multiplexor::error(){
	err++;
}

//metodo que resetea los errores
void Multiplexor::reset(){
	err = 0;
}

//verifica si hay varios errores seguidos
bool Multiplexor::verify(){
	if(err >= ERR_MAX){
		ESP_LOGE(TAG, "Multiplexor I2C ha tenido varios errores seguidos");
		rgb(0, 1023);
		return false;
	}
	return true;
}

//metodo que reinicia el i2c
void Multiplexor::reinicio(){
	//log
	ESP_LOGW(TAG, "Fallo critico, reiniciando el bus I2C para desbloquear");
	//elimina el driver
	i2c_driver_delete(I2C_MASTER_NUM);
	//vuelve a iniciar el i2c

	//configurar pines como salida
	gpio_config_t conf_o;
	conf_o.pin_bit_mask = (1ULL << I2C_MASTER_SCL_IO) | (1ULL << I2C_MASTER_SDA_IO);
	conf_o.mode = GPIO_MODE_OUTPUT_OD;
	conf_o.pull_up_en = GPIO_PULLUP_ENABLE;
	conf_o.pull_down_en = GPIO_PULLDOWN_DISABLE;
	conf_o.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&conf_o);

	//secuencia de desbloqueo
	gpio_set_level(I2C_MASTER_SDA_IO, 1);
	for(int i = 0; i < 9; i++){
		gpio_set_level(I2C_MASTER_SCL_IO, 0);
		esp_rom_delay_us(10);
		gpio_set_level(I2C_MASTER_SCL_IO, 1);
		esp_rom_delay_us(10);
	}

	gpio_set_level(I2C_MASTER_SCL_IO, 0);
	gpio_set_level(I2C_MASTER_SDA_IO, 0);
	esp_rom_delay_us(10);
	gpio_set_level(I2C_MASTER_SCL_IO, 1);
	esp_rom_delay_us(10);
	gpio_set_level(I2C_MASTER_SDA_IO, 1);
	esp_rom_delay_us(10);

	//volvemos a iniciar el i2c
	i2c_param_config(I2C_MASTER_NUM, &conf);
	i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	//resetea los errores
	reset();
	ESP_LOGI(TAG, "Bus I2C reiniciado correctamente");
}
