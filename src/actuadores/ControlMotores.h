#ifndef CONTROLMOTORES_H
#define CONTROLMOTORES_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include <cstdint>

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

		//variables de la rampa
		int tRam;

		//funcion velocidad
		void velocidad(int16_t vel_1, int16_t vel_2, bool ram);

		//variables que definien las velocidades
		int16_t vel_nI;
		int16_t vel_nD;
		int16_t vel_aI;
		int16_t vel_aD;
		int16_t vel_mI;
		int16_t vel_mD;
		int16_t vel_pI;
		int16_t vel_pD;
		int16_t vel_gI;
		int16_t vel_gD;

		//variables que guardan la velocidad actual
		int16_t vel1;
		int16_t vel2;

		//metodos de acciones
		void dir_a();
		void dir_b();
		void ataque_ai();
		void ataque_bi();
		void ataque_ad();
		void ataque_bd();
		void max_a();
		void max_b();
		void pronunciado_ai();
		void pronunciado_bi();
		void pronunciado_ad();
		void pronunciado_bd();
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
