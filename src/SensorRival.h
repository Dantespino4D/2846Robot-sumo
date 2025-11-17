#ifndef SENSORRIVAL_H
#define SENSORRIVAL_H

#define N_MUESTRAS 5
#include "driver/gpio.h"
class SensorRival{
	private:
		//pines de los sensores
		gpio_num_t trig_1;
		gpio_num_t trig_2;
		gpio_num_t echo_1;
		gpio_num_t echo_2;

		//limite de distancia
		int maxd;

		//variables que almacenan muestas
		int mem1[N_MUESTRAS];
		int mem2[N_MUESTRAS];

		//variables que de los indices
		int ind1;
		int ind2;

		//variables de los totales promedio
		long total1;
		long total2;

		//metodo para medir distancia
		uint32_t dist_cm(gpio_num_t trig_pin, gpio_num_t echo_pin);

		uint32_t filtro(gpio_num_t trig, gpio_num_t echo, int* mem, int& ind, long& total);

	public:
		//costructor
		SensorRival(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2);

		//metodos de verificacion
		bool ojos_1Verify();
		bool ojos_2Verify();
};

#endif // !SENSORRIVAL_H
