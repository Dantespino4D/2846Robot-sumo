#ifndef MULTIPLEXOR_H
#define MULTIPLEXOR_H

#include "driver/i2c.h"
#include "driver/gpio.h"

class Multiplexor{
	public:
		void begin();
		void scSel(uint8_t i);
		i2c_port_t port();
};

#endif
