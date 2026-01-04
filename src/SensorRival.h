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

		//vaiables de las distancias leidas
		uint16_t dis1;
		uint16_t dis2;

		//limite de distancia
		int maxd;

		//variables que almacenan muestas
		uint16_t mem1[N_MUESTRAS];
		uint16_t mem2[N_MUESTRAS];

		//variables que de los indices
		int ind1;
		int ind2;

		//variables de los totales promedio
		long total1;
		long total2;

		//metodo para medir distancia
		uint16_t dist_cm(gpio_num_t trig_pin, gpio_num_t echo_pin);

		uint16_t filtro(gpio_num_t trig, gpio_num_t echo, uint16_t* mem, int& ind, long& total);


	public:
		//costructor
		SensorRival(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2);

		//metodo para leer los valores del nvs
		void nvsLeer();

		//metodos de verificacion
		bool ojos_1Verify();
		bool ojos_2Verify();

		//metodo para devolver las distancias
		void distancias(uint16_t* d1, uint16_t* d2);
};

#endif // !SENSORRIVAL_H
