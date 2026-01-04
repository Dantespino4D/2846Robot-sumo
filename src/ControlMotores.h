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

		//variables que definien las velocidades
		int vel_nI;
		int vel_nD;
		int vel_aI;
		int vel_aD;
		int vel_gI;
		int vel_gD;

		//variables que guardan la velocidad actual
		int16_t vel1;
		int16_t vel2;

		//metodos de acciones
		void dir_a();
		void dir_b();
		void ataque_a();
		void ataque_b();
		void giro();

		//metodo para leer y aplicar los valores de la nvs
		void nvsLeer();
	public:
		//variables control e inicializacion
		ControlMotores(gpio_num_t motA2, gpio_num_t motB2, gpio_num_t motA1, gpio_num_t motB1);
		void begin();
		void controlador(int accion);
		//alto
		void alto();
		//envia los datos a telemetria
		void velocidades(int16_t* v1, int16_t* v2);
};

#endif
