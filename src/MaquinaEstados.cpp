#include "MaquinaEstados.h"
#include "Nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include <cstdint>

MaquinaEstados::MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int _tiempo4, TaskHandle_t* _motr):
	tempT(0),
	tempC(0),
	tempE1(0),
	tempE2(0),
	tiempo1(_tiempo1),
	tiempo2(_tiempo2),
	tiempo3(_tiempo3),
	tiempo4(_tiempo4),
	motr(_motr),
	modo(-1),
	estrategia(0),
	ciclo(0),
	ini(0),
	memo_T(0),
	memo_C(0),
	memo_E(false)
{
	nvsLeer();
}

//se cuentan kis tiempos
void MaquinaEstados::tiempo(){
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	// condiciones que evaluan si ya pasaron los tiempos
    if (temp - tempT >= tiempo1) {
		//tiempo de ataque ciego dir a derecha
    	memo_T = 0;
    }
    if (temp - tempC >= tiempo2) {
		//timepo de retroceso dir a
    	memo_C = 0;
    }
}

// selecciona el estado del prototipo
void MaquinaEstados::seleccionP(){
	uint32_t noti;
	xTaskNotifyWait(0x00, 0xFFFFFFFF, &noti, 10);
	// si detecta el limite por sc_1
	if (noti & (1 << 0)) {
  		modo = 0;
	}
	// si detecta el limite por sc_2
	else if (noti & (1 << 1)) {
	 	 modo = 1;
	}
	// si deja de detectar el limite por sc 1
	else if (memo_C == 1) {
  		modo = 2;
	}
	// si deja de detectar el limite por sc 2
	else if (memo_C == 2) {
		modo = 3;
    }
	// si detecta el robot por ojos 1
	else if (noti & (1 << 2)) {
  		modo = 4;
	}
    // si detecta el robot por ojos 2
	else if (noti & (1 << 3)) {
		modo = 5;
	}
	// si deja de detectar al robot por ojos 1
	else if (memo_T == 1) {
  		modo = 6;
	}
	// si deja de detectar al robot por ojos 2
	else if (memo_T == 2) {
		modo = 7;
    }
    // si no detecta nada
	else if(memo_E){
		modo = 8;
    }
	else{
		modo = 9;
	}
}

//selecciona el estado
void MaquinaEstados::seleccion(){
	uint32_t noti;
	xTaskNotifyWait(0x00, 0xFFFFFFFF, &noti, 10);
	// si detecta el limite por sc_1
	if (noti & (1 << 0)) {
		modo = 0;
	}
	// si detecta el limite por sc_2
	else if (noti & (1 << 1)) {
	 	 modo = 1;
	}
	// si deja de detectar el limite por sc 1
	else if (memo_C == 1) {
		modo = 2;
	}
	// si deja de detectar el limite por sc 2
	else if (memo_C == 2) {
		modo = 3;
	}
	//si detecta al robot por ToF1, ToF2 y ToF3
	else if((noti & (1 << 2)) && (noti & (1 << 3)) && (noti & (1 << 4))) {
		modo = 4;
	}
	//si detecta al robot por ToF1 y Tof2
	else if((noti & (1 << 2)) && (noti & (1 << 3))) {
		modo = 5;
	}
	//si detecta al robot por ToF1 y ToF3
	else if((noti & (1 << 2)) && (noti & (1 << 4))) {
		modo = 6;
	}
	//si detecta al robot por ToF2 y ToF3
	else if((noti & (1 << 3)) && (noti & (1 << 4))) {
		modo = 7;
	}
	// si detecta el robot por ToF1
	else if (noti & (1 << 2)) {
		modo = 8;
	}
	// si detecta el robot por ToF2
	else if (noti & (1 << 3)) {
		modo = 9;
	}
	//si detecta al robot por ToF3
	else if (noti & (1 << 4)) {
	  	modo = 10;
	}
	//si detecta al robot por ToF4, ToF5 y ToF6
	else if((noti & (1 << 5)) && (noti & (1 << 6)) && (noti & (1 << 7))) {
		modo = 11;
	}
	//si detecta al robot por ToF4 y ToF5
	else if((noti & (1 << 5)) && (noti & (1 << 6))) {
		modo = 12;
	}
	//si detecta al robot por ToF4 y ToF6
	else if((noti & (1 << 5) && (noti & (1 << 7)))){
		modo = 13;
	}
	//si detecta al robot por ToF5 y ToF6
	else if((noti & (1 << 6)) && (noti & (1 << 7))) {
		modo = 14;
	}
	// si detecta al robot por ToF4
	else if (noti & (1 << 5)) {
	  	modo = 15;
	}
	// si detecta al robot por ToF5
	else if (noti & (1 << 6)) {
		modo = 16;
	}
	//si detecta al robot por ToF6
	else if (noti & (1 << 7)) {
		modo = 17;
	}
	else if (memo_T == 1) {
	  		modo = 18;
	}
	else if(memo_T == 2) {
	  		modo = 19;
	}
	else if (memo_T == 3) {
		modo = 20;
	}
	else if (memo_T == 4) {
	  		modo = 21;
	}
	else if (memo_E) {
		modo = 22;
	}
	// si no detecta nada
	else{
		modo = 23;
	}
}

//la ejecucion de la estrategia del prototipo
void MaquinaEstados::ejecucionEP(){
	// variable del comandos
    int com = ALTO;

	//se guarda el tiempo
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

    // ejecuta el estado
    switch (modo) {
		// detiene el movimiento y retrocede en direccion b
    	case 0:
      		com = DIR_B;
      		memo_C = 1;
      		tempC = temp;
      		break;
      	// detiene el movimiento y retrocede en direccion a
    	case 1:
    	  	com = DIR_A;
      		memo_C = 0;
      		tempC = temp;
      		break;
		// continua avanzando en direccion b por un tiempo definido para alejarse del borde
    	case 2:
      		com = DIR_B;
      		break;
    	// continua avanzando en direccion a por un tiempo definido para alejarse del borde
    	case 3:
      		com = DIR_A;
      		break;
    	// avanza en direccion a
    	case 4:
      		com = ATAQUE_AI;
      		memo_T = 1;
      		tempT = temp;
      		break;
    	// avanza en direccion b
    	case 5:
      		com = ATAQUE_BI;
      		memo_T = 2;
      		tempT = temp;
      		break;
    	// avanza por un tiempo definido de 4 segundo en direccion a
    	case 6:
      		com = ATAQUE_AI;
      		break;
    	// avanza por un tiempo definido de 4 segundos en direccion b
    	case 7:
      		com = ATAQUE_BI;
      		break;
      	// da vueltas hasta encontrar el robot
    	case 8:
      		//aqui pondre la logica para que avanze formando la estrella
			if (temp - tempE1 >= tiempo3) {
    			memo_E = false;        // ir a girar
    			tempE2 = temp;
				com = GIRO;
			}else{
      			com = DIR_A;
			}
      		break;
		case 9:
    		if(temp - tempE2 >= tiempo4){
    		    memo_E = true;      // Termina giro → pasará a estado 8
    		    tempE1 = temp;      // Reiniciar timer del avance
				com = DIR_A;
    		} else {
    		    com = GIRO;
    		}
            break;
		default:
			com = ALTO;
			break;
	}
    xTaskNotify(*motr, com, eSetValueWithOverwrite);
}

//ejecucion de la estrategia 1
void MaquinaEstados::ejecucionE1(){
	//variable del comandos
	int com = ALTO;

	//variable que guarda el tiempo
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	switch (modo) {
		// detiene el movimiento y retrocede en direccion b
		case 0:
	  		com = DIR_B;
	  		memo_C = 1;
	  		tempC = temp;
	  		break;
	  	// detiene el movimiento y retrocede en direccion a
		case 1:
		  	com = DIR_A;
	  		memo_C = 2;
	  		tempC = temp;
	  		break;
		// continua avanzando en direccion b por un tiempo definido para alejarse del borde
		case 2:
	  		com = DIR_B;
	  		break;
		// continua avanzando en direccion a por un tiempo definido para alejarse del borde
		case 3:
	  		com = DIR_A;
	  		break;
         // enemigo inmnente direccion a
        case 4:
              com = MAX_A;
              break;
        //enemigo detectado en direccion a izquierda
        case 5:
			memo_T = 1;
			tempT = temp;
            com = ATAQUE_AI;
			break;
        //incoherencia, posible falla de los sensores en direccion a(accion por definir que es mejor)
        case 6:
            com = MAX_A;
            break;
        // enemigo detectado ne direccion a derecha
        case 7:
			memo_T = 2;
			tempT = temp;
            com = ATAQUE_AD;
            break;
        // enemigo detectado muy a la izquierda en direccion a
        case 8:
			memo_T = 1;
			tempT = temp;
            com = PRO_AI;
			break;
        //enemigo detectado lejos en direccion a
        case 9:
            com = DIR_A;
            break;
        // enemigo detectado muy a la derecha en direccion a
        case 10:
			memo_T = 2;
			tempT = temp;
            com = PRO_AD;
            break;
        // enemigo inminente direccio a
        case 11:
            com = MAX_A;
            break;
        // enemigo detectado en direccion b izquierda
        case 12:
			memo_T = 3;
			tempT = temp;
            com = ATAQUE_BI;
            break;
        //incoherencia, posible falla de los sensores en direccion b(accion por definir que es mejor)
        case 13:
            com = MAX_A;
            break;
        // enemigo detectado en direccion b derecha
        case 14:
			memo_T = 4;
			tempT = temp;
            com = ATAQUE_BD;
            break;
        // enemigo detectado muy a la izquierda en direccion b
        case 15:
			memo_T = 3;
			tempT = temp;
            com = PRO_BI;
            break;
        // enemigo detectado lejos en direccion b
        case 16:
            com = DIR_B;
            break;
        // enemigo detectado muy a la derecha en direccion b
        case 17:
			memo_T = 4;
			tempT = temp;
            com = PRO_BD;
            break;
		//memoria del sector AI
		case 18:
			com = PRO_AI;
			break;
		//memoria del sector AD
		case 19:
			com = PRO_AD;
			break;
		//memoria del sector BI
		case 20:
			com = PRO_BI;
			break;
		//memoria del sector BD
		case 21:
			com = PRO_BD;
			break;
		//avanze de estrella
		case 22:
			//logica para que avanze formando la estrella
			if (temp - tempE1 >= tiempo3) {
    			memo_E = false;        // ir a girar
    			tempE2 = temp;         // inicia tiempo del giro
				com = GIRO;
			}else{
      			com = DIR_A;
			}
      		break;
		//giro de estrella
		case 23:
    		if(temp - tempE2 >= tiempo4){
    		    memo_E = true;      // Termina giro → pasará a estado 8
    		    tempE1 = temp;      // Reiniciar timer del avance
				com = DIR_A;
    		} else {
    		    com = GIRO;
    		}
            break;
        default:
            com = ALTO;
            break;
    }
    xTaskNotify(*motr, com, eSetValueWithOverwrite);
}


void MaquinaEstados::ejecucionE2(){
	//variable del comandos
	int com = ALTO;

	//variable que guarda el tiempo
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	switch (modo) {
		// detiene el movimiento y retrocede en direccion b
		case 0:
	  		com = DIR_B;
	  		memo_C = 1;
	  		tempC = temp;
	  		break;
	  	// detiene el movimiento y retrocede en direccion a
		case 1:
		  	com = DIR_A;
	  		memo_C = 2;
	  		tempC = temp;
	  		break;
		// continua avanzando en direccion b por un tiempo definido para alejarse del borde
		case 2:
	  		com = DIR_B;
	  		break;
		// continua avanzando en direccion a por un tiempo definido para alejarse del borde
		case 3:
	  		com = DIR_A;
	  		break;
         // enemigo inmnente direccion a
        case 4:
              com = MAX_A;
              break;
        //enemigo detectado en direccion a izquierda
        case 5:
			memo_T = 1;
			tempT = temp;
            com = ATAQUE_AI;
			break;
        //incoherencia, posible falla de los sensores en direccion a(accion por definir que es mejor)
        case 6:
            com = MAX_A;
            break;
        // enemigo detectado ne direccion a derecha
        case 7:
			memo_T = 2;
			tempT = temp;
            com = ATAQUE_AD;
            break;
        // enemigo detectado muy a la izquierda en direccion a
        case 8:
			memo_T = 1;
			tempT = temp;
            com = PRO_AI;
			break;
        //enemigo detectado lejos en direccion a
        case 9:
            com = DIR_A;
            break;
        // enemigo detectado muy a la derecha en direccion a
        case 10:
			memo_T = 2;
			tempT = temp;
            com = PRO_AD;
            break;
        // enemigo inminente direccio a
        case 11:
            com = MAX_A;
            break;
        // enemigo detectado en direccion b izquierda
        case 12:
			memo_T = 3;
			tempT = temp;
            com = ATAQUE_BI;
            break;
        //incoherencia, posible falla de los sensores en direccion b(accion por definir que es mejor)
        case 13:
            com = MAX_A;
            break;
        // enemigo detectado en direccion b derecha
        case 14:
			memo_T = 4;
			tempT = temp;
            com = ATAQUE_BD;
            break;
        // enemigo detectado muy a la izquierda en direccion b
        case 15:
			memo_T = 3;
			tempT = temp;
            com = PRO_BI;
            break;
        // enemigo detectado lejos en direccion b
        case 16:
            com = DIR_B;
            break;
        // enemigo detectado muy a la derecha en direccion b
        case 17:
			memo_T = 4;
			tempT = temp;
            com = PRO_BD;
            break;
		//memoria del sector AI
		case 18:
			com = PRO_AD;
			break;
		//memoria del sector AD
		case 19:
			com = PRO_AI;
			break;
		//memoria del sector BI
		case 20:
			com = PRO_BD;
			break;
		//memoria del sector BD
		case 21:
			com = PRO_BI;
			break;
		//avanze de estrella
		case 22:
			//logica para que avanze formando la estrella
			if (temp - tempE1 >= tiempo3) {
    			memo_E = false;        // ir a girar
    			tempE2 = temp;         // inicia tiempo del giro
				com = GIRO;
			}else{
      			com = DIR_A;
			}
      		break;
		//giro de estrella
		case 23:
    		if(temp - tempE2 >= tiempo4){
    		    memo_E = true;      // Termina giro → pasará a estado 8
    		    tempE1 = temp;      // Reiniciar timer del avance
				com = DIR_A;
    		} else {
    		    com = GIRO;
    		}
            break;
        default:
            com = ALTO;
            break;
    }
    xTaskNotify(*motr, com, eSetValueWithOverwrite);
}

void MaquinaEstados::logica(){
	//evaluacion de tiempos
	tiempo();
	if(estrategia == 0){
		//seleccion de estado del prototipo
		seleccionP();
	}else{
		//seleccion de estado
		seleccion();
	}

	//se ejecuta dependiendo de la estrategia
	switch (estrategia) {
		case 0:
			ejecucionEP();
			break;
		case 1:
			ejecucionE1();
			break;
		case 2:
			ejecucionE2();
			break;

	}
}

void MaquinaEstados::nvsLeer(){
	Nvs nvs("tiempos");
	tiempo1 = nvs.leer("ataque_ciego",tiempo1);
	tiempo2 = nvs.leer("retroceso",tiempo2);
	tiempo3 = nvs.leer("recta_star",tiempo3);
	tiempo4 = nvs.leer("giro_star",tiempo4);
	estrategia = nvs.leer("estrategia",estrategia);
}

//metodo que recibe la duracion del ciclo
void MaquinaEstados::cicloR(int c, int i){
	ciclo = c;
	ini = i;
}

//metodo que entrega el estado actual
void MaquinaEstados::datos(int* _modo, int* _estra, int* _ciclo, int* _ini){
	 *_modo = modo;
	 *_estra = estrategia;
	 *_ciclo =ciclo;
	 *_ini = ini;
}
