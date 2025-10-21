#ifndef CONTROLMOTORES_H
#define CONTROLMOTORES_H

#include <Arduino.h>

class ControlMotores{
	private:
		//variables necesarias
		int freq;
		int solut;
		int pwmC_1;
		int pwmC_2;
		int pwm_1;
		int pwm_2;
		int mot[2][2];

		//metodos de acciones
		void alto();
		void dir_a();
		void dir_b();
		void giro();

	public:
		//variables control e inicializacion
		ControlMotores(int _pwm_1, int _pwm_2, int motA_1, int motA_2, int motB_1, int motB_2);
		void begin();
		void controlador(int accion);
};

#endif
