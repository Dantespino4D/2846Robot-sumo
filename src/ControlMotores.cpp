#include <Arduino.h>
#include "ControlMotores.h"

ControlMotores::ControlMotores(int _pwm_1, int _pwm_2, int motA_1, int motA_2, int motB_1, int motB_2):
// Valores de configuracion pwm
    freq(5000),
    solut(8),
    pwmC_1(0),
    pwmC_2(1),

    // Pines pwm
    pwm_1(_pwm_1),
    pwm_2(_pwm_2),

	//pines de los motores
	mot{ {motA_1, motA_2}, {motB_1, motB_2} }
{}

void ControlMotores::alto(){
	digitalWrite(mot[0][0], LOW);
  	digitalWrite(mot[0][1], LOW);
  	digitalWrite(mot[1][0], LOW);
  	digitalWrite(mot[1][1], LOW);
}

void ControlMotores::dir_a(){
	alto();
    ledcWrite(pwmC_1, 255);
    ledcWrite(pwmC_2, 255);
  	digitalWrite(mot[0][0], HIGH);
  	digitalWrite(mot[1][0], HIGH);
}

void ControlMotores::dir_b(){
	alto();
    ledcWrite(pwmC_1, 255);
    ledcWrite(pwmC_2, 255);
  	digitalWrite(mot[0][1], HIGH);
  	digitalWrite(mot[1][1], HIGH);

}

void ControlMotores::giro(){
	alto();
    ledcWrite(pwmC_1, 255);
    ledcWrite(pwmC_2, 255);
  	digitalWrite(mot[0][1], HIGH);
  	digitalWrite(mot[1][0], HIGH);
}

void ControlMotores::begin(){
	//se inicializan los pines
	for(int i = 0; i < 2; i++){
		for(int j = 0; j < 2; j++){
			pinMode(mot[i][j], OUTPUT);
		}
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
