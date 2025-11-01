#include <Arduino.h>
#ifndef CONTROLMOTORES_H
#define CONTROLMOTORES_H

#include "driver/ledc.h"

class ControlMotores{
	private:
		//variables necesarias
		int freq;
		int solut;
		int pwmC_1;
		int pwmC_2;
		int pwm_1;
		int pwm_2;
		int mot[2];

		//funcion velocidad
		void velocidad(int vel_1, int vel_2);

		//metodos de acciones
		void dir_a();
		void dir_b();
		void giro();

	public:
		//variables control e inicializacion
		ControlMotores(int _pwm_1, int _pwm_2, int motA, int motB);
		void begin();
		void controlador(int accion);
		//alto
		void alto();
};

#endif
