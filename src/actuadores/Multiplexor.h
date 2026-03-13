#ifndef MULTIPLEXOR_H
#define MULTIPLEXOR_H

#include "driver/i2c.h"
#include "driver/gpio.h"

#define ERR_MAX 10

class Multiplexor{
	private:
		int err;
		i2c_config_t conf;
	public:
		void begin();
		void sel(uint8_t i);
		i2c_port_t port();
		void error();
		void reset();
		bool verify();
		void reinicio();
};

#endif
