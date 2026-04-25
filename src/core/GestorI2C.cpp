#include "GestorI2C.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "../actuadores/rgb.h"
#include "../configuracion/pines.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

static const char* TAG = "GestorI2C";

GestorI2C::GestorI2C() :
	err(0)
{}

void GestorI2C::begin() {
    conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA;
    conf.scl_io_num = SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    esp_err_t res = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al instalar I2C: %s", esp_err_to_name(res));
        rgb(0, 1023);
    }
}

void GestorI2C::error() {
	err++;
}

void GestorI2C::reset() {
	err = 0;
}

i2c_port_t GestorI2C::port() const {
	return I2C_MASTER_NUM;
}

bool GestorI2C::verify(){
	if(err >= ERR_MAX){
		ESP_LOGE(TAG, "Multiplexor I2C ha tenido varios errores seguidos");
		rgb(0, 1023);
		reinicio();
		return false;
	}
	return true;
}

//metodo que reinicia el i2c
void GestorI2C::reinicio(){
	//log
	ESP_LOGW(TAG, "Fallo critico, reiniciando el bus I2C para desbloquear");
	//elimina el driver
	i2c_driver_delete(I2C_MASTER_NUM);
	//vuelve a iniciar el i2c

	//configurar pin del scl
	gpio_config_t conf_scl;
	conf_scl.pin_bit_mask = (1ULL << SCL);
	conf_scl.mode = GPIO_MODE_OUTPUT_OD;
	conf_scl.pull_up_en = GPIO_PULLUP_ENABLE;
	conf_scl.pull_down_en = GPIO_PULLDOWN_DISABLE;
	conf_scl.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&conf_scl);

	gpio_config_t conf_sda;
	conf_sda.pin_bit_mask = (1ULL << SDA);
	conf_sda.mode = GPIO_MODE_INPUT_OUTPUT_OD;
	conf_sda.pull_up_en = GPIO_PULLUP_ENABLE;
	conf_sda.pull_down_en = GPIO_PULLDOWN_DISABLE;
	conf_sda.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&conf_sda);


	//secuencia de desbloqueo
	gpio_set_level((gpio_num_t)SDA, 1);
    gpio_set_level((gpio_num_t)SCL, 1);

	bool desbloqueado = false;

	for(int i = 0; i < 9; i++){
		if(gpio_get_level((gpio_num_t)SDA) == 1){
			desbloqueado = true;
			break;
		}
		gpio_set_level(SCL, 0);
		esp_rom_delay_us(10);
		gpio_set_level(SCL, 1);
		esp_rom_delay_us(10);
	}

	gpio_set_level(SCL, 0);
	gpio_set_level(SDA, 0);
	esp_rom_delay_us(10);
	gpio_set_level(SCL, 1);
	esp_rom_delay_us(10);
	gpio_set_level(SDA, 1);
	esp_rom_delay_us(10);

	if(!desbloqueado &&  gpio_get_level((gpio_num_t)SDA) == 0){
		ESP_LOGE(TAG, "daño fisico");
		rgb(1023, 0);
		return;
	}else {
		ESP_LOGI(TAG, "bus desbloqueado");
		reset();
	}
	begin();
}
