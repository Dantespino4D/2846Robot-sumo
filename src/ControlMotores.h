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
		gpio_num_t pwm_1;
		gpio_num_t pwm_2;
		gpio_num_t mot[2][2];

		//funcion velocidad
		void velocidad(int vel_1, int vel_2);

		//metodos de acciones
		void dir_a();
		void dir_b();
		void giro();

	public:
		//variables control e inicializacion
		ControlMotores(gpio_num_t _pwm_1, gpio_num_t _pwm_2, gpio_num_t motA_1, gpio_num_t motA_2, gpio_num_t motB_1, gpio_num_t motB_2);
		void begin();
		void controlador(int accion);
		//alto
		void alto();
};

#endif
