#ifndef SENSORRIVAL_H
#define SENSORRIVAL_H

#include "driver/gpio.h"
class SensorRival{
	private:
		//pines de los sensores
		gpio_num_t trig_1;
		gpio_num_t trig_2;
		gpio_num_t echo_1;
		gpio_num_t echo_2;

		//metodo para medir distancia
		uint32_t dist_cm(gpio_num_t trig_pin, gpio_num_t echo_pin);

		//limite de distancia
		int maxd;

	public:
		//costructor
		SensorRival(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2);

		//metodos de verificacion
		bool ojos_1Verify();
		bool ojos_2Verify();
};

#endif // !SENSORRIVAL_H
