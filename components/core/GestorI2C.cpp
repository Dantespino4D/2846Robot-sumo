#include "GestorI2C.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "rgb.h"
#include "pines.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

static const char* TAG = "GestorI2C";

GestorI2C::GestorI2C() :
    bus_handle(nullptr),
	err(0)
{}

void GestorI2C::begin() {
    i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_MASTER_NUM;
    bus_config.sda_io_num = (gpio_num_t)SDA;
    bus_config.scl_io_num = (gpio_num_t)SCL;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    esp_err_t res = i2c_new_master_bus(&bus_config, &bus_handle);

    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al instalar I2C Bus: %s", esp_err_to_name(res));
        rgb(0, 1023);
    } else {
		ESP_LOGI(TAG, "I2C Bus instalado correctamente");
    }
}

i2c_master_dev_handle_t GestorI2C::get_device(uint8_t addr) {
    if (devices.find(addr) != devices.end()) {
        return devices[addr];
    }

    i2c_device_config_t dev_config = {};
    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.device_address = addr;
    dev_config.scl_speed_hz = I2C_MASTER_FREQ_HZ;

    i2c_master_dev_handle_t dev_handle;
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (err == ESP_OK) {
        devices[addr] = dev_handle;
        return dev_handle;
    } else {
        ESP_LOGE(TAG, "Error al añadir dispositivo 0x%02X: %s", addr, esp_err_to_name(err));
        return nullptr;
    }
}

void GestorI2C::error() {
	err++;
}

void GestorI2C::reset() {
	err = 0;
}

i2c_master_bus_handle_t GestorI2C::get_bus() const {
	return bus_handle;
}

bool GestorI2C::verify(){
	if(err >= ERR_MAX){
		ESP_LOGE(TAG, "Gestor I2C ha tenido varios errores seguidos");
		rgb(0, 1023);
		reinicio();
		return false;
	}
	return true;
}

void GestorI2C::reinicio(){
	ESP_LOGW(TAG, "Fallo critico, reiniciando el bus I2C para desbloquear");
	
    // Limpiar dispositivos y bus
    for (auto const& [addr, handle] : devices) {
        i2c_master_bus_rm_device(handle);
    }
    devices.clear();
    
    if (bus_handle) {
        i2c_del_master_bus(bus_handle);
        bus_handle = nullptr;
    }

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
		gpio_set_level((gpio_num_t)SCL, 0);
		esp_rom_delay_us(10);
		gpio_set_level((gpio_num_t)SCL, 1);
		esp_rom_delay_us(10);
	}

	gpio_set_level((gpio_num_t)SCL, 0);
	gpio_set_level((gpio_num_t)SDA, 0);
	esp_rom_delay_us(10);
	gpio_set_level((gpio_num_t)SCL, 1);
	esp_rom_delay_us(10);
	gpio_set_level((gpio_num_t)SDA, 1);
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
