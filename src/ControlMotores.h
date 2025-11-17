#ifndef CONTROLMOTORES_H
#define CONTROLMOTORES_H

#include "driver/gpio.h"
#include "driver/ledc.h"

class ControlMotores{
	private:
		//variables necesarias
		int freq;
		ledc_timer_bit_t solut;
		ledc_channel_t pwmC_1;
		ledc_channel_t pwmC_2;
		ledc_channel_t pwmC_3;
		ledc_channel_t pwmC_4;

		gpio_num_t mot2[2];
		gpio_num_t mot[2];

		//funcion velocidad
		void velocidad(int vel_1, int vel_2);

		//metodos de acciones
		void dir_a();
		void dir_b();
		void ataque_a();
		void ataque_b();
		void giro();

	public:
		//variables control e inicializacion
		ControlMotores(gpio_num_t motA2, gpio_num_t motB2, gpio_num_t motA1, gpio_num_t motB1);
		void begin();
		void controlador(int accion);
		//alto
		void alto();
};

#endif
