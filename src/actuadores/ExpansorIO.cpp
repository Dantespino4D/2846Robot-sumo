#include "ExpansorIO.h"
#include "../core/GestorI2C.h"
#include "driver/i2c.h"

ExpansorIO::ExpansorIO(GestorI2C& _i2c) :
	direccion(0x20),
	estado(0),
	i2c(_i2c)
{}

void ExpansorIO::escribir(uint8_t pin, uint8_t value) {
	if (value) {
		estado |= (1 << pin);
	} else {
		estado &= ~(1 << pin);
	}
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (direccion << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, estado, true);
	i2c_master_stop(cmd);
	esp_err_t res = i2c_master_cmd_begin(i2c.port(), cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	if (res != ESP_OK) {
		i2c.error();
	}
}
