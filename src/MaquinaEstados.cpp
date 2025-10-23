#include <Arduino.h>
#include "MaquinaEstados.h"

MaquinaEstados::MaquinaEstados(int _tiempo1, int _tiempo2,SemaphoreHandle_t& _alerta, SemaphoreHandle_t& _alerta2,
                   SemaphoreHandle_t& _enemigo, SemaphoreHandle_t& _enemigo2,
                   QueueHandle_t& _orden):
	temp1(0),
	temp2(0),
	temp3(0),
	temp4(0),
	tiempo1(_tiempo1),
	tiempo2(_tiempo2),
	alerta(_alerta),
	alerta2(_alerta2),
	enemigo(_enemigo),
	enemigo2(_enemigo2),
	orden(_orden),
	memo1(false),
	memo2(false),
	memo3(false),
	memo4(false),
	modo(6)
{}

//se cuentan kis tiempos
void MaquinaEstados::tiempo(){
	// condiciones que evaluan si ya pasaron los tiempos
    if (millis() - temp1 >= tiempo1) {
    	memo1 = false;
    }
    if (millis() - temp2 >= tiempo1) {
      	memo2 = false;
    }
    if (millis() - temp3 >= tiempo2) {
    	memo3 = false;
    }
    if (millis() - temp4 >= tiempo2) {
     	memo4 = false;
    }
}

// selecciona el estado
void MaquinaEstados::seleccion(){
	// si detecta el limite por sc_1
    if (xSemaphoreTake(alerta, 0) == pdTRUE || memo3) {
  		modo = 0;
	}
	// si detecta el limite por sc_2
	else if (xSemaphoreTake(alerta2, 0) == pdTRUE || memo4) {
	 	 modo = 1;
	}
	// si deja de detectar al robot por ojos 1
	else if (xSemaphoreTake(enemigo, 0)) {
  		modo = 2;
	}
    // si deja de detectar al robot por ojos 2
	else if (xSemaphoreTake(enemigo2, 0)) {
		modo = 3;
	}
	// si detecta el robot por ojos 1
	else if (memo1) {
  		modo = 4;
	}
	// si detecta el robot por ojos 2
	else if (memo2) {
		modo = 5;
    }
    // si no detecta nada
    else {
		modo = 6;
    }
}

void MaquinaEstados::ejecucion(){
	// variable del comandos
    int com;

    // ejecuta el estado
    switch (modo) {
    // detiene el movimiento y retrocede en direccion b
    	case 0:
      		com = DIR_B;
      		xQueueSend(orden, &com, 0);
      		memo3 = true;
      		temp4 = millis();
      		break;
      	// detiene el movimiento y retrocede en direccion a
    	case 1:
    	  	com = DIR_A;
      		xQueueSend(orden, &com, 0);
      		memo4 = true;
      		temp3 = millis();
      		break;
    	// avanza por un tiempo definido de 4 segundo en direccion a
    	case 2:
      		com = DIR_A;
      		xQueueSend(orden, &com, 0);
      		memo1 = true;
      		temp1 = millis();
      		temp3 = millis();
      		temp4 = millis();
      		break;
    	// avanza por un tiempo definido de 4 segundos en direccion b
    	case 3:
      		com = DIR_B;
      		xQueueSend(orden, &com, 0);
      		memo2 = true;
      		temp2 = millis();
      		temp3 = millis();
      		temp4 = millis();
      		break;
    	// avanza en direccion a
    	case 4:
      		com = DIR_A;
      		xQueueSend(orden, &com, 0);
      		temp3 = millis();
      		temp4 = millis();
      		break;
    	// avanza en direccion b
    	case 5:
      		com = DIR_B;
      		xQueueSend(orden, &com, 0);
      		temp3 = millis();
      		temp4 = millis();
      		break;
      	// da vueltas hasta encontrar el robot
    	case 6:
      		com = GIRO;
      		xQueueSend(orden, &com, 0);
      		temp3 = millis();
      		temp4 = millis();
      		break;
	}
}

void MaquinaEstados::logica(){
	//evaluacion de tiempos
	tiempo();

	//seleccion de estado
	seleccion();

	//ejecucion del estado
	ejecucion();
}
