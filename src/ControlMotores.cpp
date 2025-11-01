#include <Arduino.h>
#include "driver/ledc.h"
#include "ControlMotores.h"

ControlMotores::ControlMotores(int _pwm_1, int _pwm_2, int motA, int motB):
// Valores de configuracion pwm
    freq(5000),
    solut(8),
    pwmC_1(0),
    pwmC_2(1),

    // Pines pwm
    pwm_1(_pwm_1),
    pwm_2(_pwm_2),

	//pines de los motores
	mot{motA, motB}
{}

//estblecer velocidad
void ControlMotores::velocidad(int vel_1, int vel_2){
    ledcWrite(pwmC_1, vel_1);
    ledcWrite(pwmC_2, vel_2);
}

void ControlMotores::alto(){
	digitalWrite(mot[0], LOW);
  	digitalWrite(mot[1], LOW);
	velocidad(0, 0);
	delayMicroseconds(100);
}

void ControlMotores::dir_a(){
	alto();
  	digitalWrite(mot[1], HIGH);
  	digitalWrite(mot[1], HIGH);
	velocidad(255, 255);
}

void ControlMotores::dir_b(){
	alto();
	digitalWrite(mot[0], HIGH);
  	digitalWrite(mot[0], HIGH);
	velocidad(255, 255);

}

void ControlMotores::giro(){
	alto();
	digitalWrite(mot[0], HIGH);
  	digitalWrite(mot[1], HIGH);
	velocidad(255, 255);
}

void ControlMotores::begin(){
	//se inicializan los pines
	for(int i = 0; i < 2; i++){
		pinMode(mot[i], OUTPUT);
	}

  	// configuracion y asignacion de los pines pwm
  	ledcSetup(pwmC_1, freq, solut);
  	ledcSetup(pwmC_2, freq, solut);
  	ledcAttachPin(pwm_1, pwmC_1);
  	ledcAttachPin(pwm_2, pwmC_2);
}

void ControlMotores::controlador(int accion){
	switch(accion){
		case 0:
			alto();
			break;
		case 1:
			dir_a();
			break;
		case 2:
			dir_b();
			break;
		case 3:
			giro();
			break;
	}
}
