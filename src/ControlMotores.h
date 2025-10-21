#ifndef CONTROLMOTORES_H
#define CONTROLMOTORES_H
#include <Arduino.h>

class ControlMotores{
	private:
		int freq;
		int solut;
		int pwmC_1;
		int pwmC_2;
		int pwm_1;
		int pwm_2;
		int mot[2][2];
}
#endif // !DEBUG
