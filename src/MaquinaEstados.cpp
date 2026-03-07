#include "MaquinaEstados.h"
#include "Nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include <cstdint>

MaquinaEstados::MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int _tiempo4, int _tiempo5, TaskHandle_t* _motr):
	tempTL(0),
	tempC(0),
	tempE1(0),
	tempE2(0),
	tiempo1(_tiempo1),
	tiempo2(_tiempo2),
	tiempo3(_tiempo3),
	tiempo4(_tiempo4),
	tiempo5(_tiempo5),
	motr(_motr),
	modo(-1),
	estrategia(0),
	ciclo(0),
	ini(0),
	memo_TC(0),
	memo_TL(0),
	memo_C(0),
	memo_E(false)
{
	nvsLeer();
}

//se cuentan kis tiempos
void MaquinaEstados::tiempo(){
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	// condiciones que evaluan si ya pasaron los tiempos
    if (temp - tempC >= tiempo1) {
		//timepo de retroceso dir a
    	memo_C = 0;
    }
	if (temp - tempTC >= tiempo2) {
		//tiempo de memoria a corto plazo
		memo_TC = 0;
	}
    if (temp - tempTL >= tiempo3) {
		//tiempo de la memoria a largo plazo
    	memo_TL = 0;
    }

}

// selecciona el estado del prototipo
void MaquinaEstados::seleccionP(){
	uint32_t noti = 0; // INICIALIZADO A 0 PARA EVITAR BASURA
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
	else if (memo_TL == 1) {
  		modo = 6;
	}
	// si deja de detectar al robot por ojos 2
	else if (memo_TL == 2) {
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
	uint32_t noti = 0; // INICIALIZADO A 0 PARA EVITAR BASURA
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
	//si deja de detectar al robot por direccion AI_P a corto plazo
	else if (memo_TC == 1) {
		modo = 18;
	}
	//si deja de detectar al robot por direccion AD_P a corto plazo
	else if (memo_TC == 2) {
		modo = 19;
	}
	//si deja de detectar al robot por direccion BI_P a corto plazo
	else if (memo_TC == 3) {
		modo = 20;
	}
	//si deja de detectar al robot por direccion BD_P a corto plazo
	else if (memo_TC == 4) {
		modo = 21;
	}
	//si deja de detectar al robot por direccion AI_S a corto plazo
	else if (memo_TC == 5) {
		modo = 22;
	}
	//si deja de detectar al robot por direccion AD_S a corto plazo
	else if (memo_TC == 6) {
		modo = 23;
	}
	//si deja de detectar al robot por direccion BI_S a corto plazo
	else if (memo_TC == 7) {
		modo = 24;
	}
	//si deja de detectar al robot por direccion BD_S a corto plazo
	else if (memo_TC == 8) {
		modo = 25;
	}
	//si deja de detectar al robot de frente a corto plazo
	else if (memo_TC == 9) {
		modo = 26;
	}
	//si deja de detectar al robot por atras a corto plazo
	else if (memo_TC == 10) {
		modo = 27;
	}
	//si deja de detectar al robot por direccion AI a largo plazo
	else if (memo_TL == 1) {
	  	modo = 28;
	}
	//si deja de detectar al robot por direccion AD a largo plazo
	else if(memo_TL == 2) {
	  	modo = 29;
	}
	//si deja de detectar al robot por direccion BI a largo plazo
	else if (memo_TL == 3) {
		modo = 30;
	}
	//si deja de detectar al robot por direccion BD a largo plazo
	else if (memo_TL == 4) {
	  	modo = 31;
	}
	//memoria de avance de estrella
	else if (memo_E) {
		modo = 32;
	}
	// si no detecta nada
	else{
		modo = 33;
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
    	// avanza en direccion a
    	case 4:
      		com = ATAQUE_AI;
      		memo_TL = 1;
      		tempTL = temp;
      		break;
    	// avanza en direccion b
    	case 5:
      		com = ATAQUE_BI;
      		memo_TL = 2;
      		tempTL = temp;
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
			if (temp - tempE1 >= tiempo4) {
    			memo_E = false;        // ir a girar
    			tempE2 = temp;
				com = GIRO;
			}else{
      			com = DIR_A;
			}
      		break;
		case 9:
    		if(temp - tempE2 >= tiempo5){
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
			memo_TC = 9;
			tempTC = temp;
            break;
        //enemigo detectado en direccion a izquierda
        case 5:
            com = ATAQUE_AI;
			memo_TC = 5;
			tempTC = temp;
			break;
        //incoherencia, posible falla de los sensores en direccion a(accion por definir que es mejor)
        case 6:
            com = MAX_A;
			memo_TC = 9;
			tempTC = temp;
            break;
        // enemigo detectado ne direccion a derecha
        case 7:
            com = ATAQUE_AD;
			memo_TC = 6;
			tempTC = temp;
            break;
        // enemigo detectado muy a la izquierda en direccion a
        case 8:
            com = PRO_AI;
			memo_TC = 1;
			tempTC = temp;
			break;
        //enemigo detectado lejos en direccion a
        case 9:
            com = DIR_A;
			memo_TC = 9;
			tempTC = temp;
            break;
        // enemigo detectado muy a la derecha en direccion a
        case 10:
            com = PRO_AD;
			memo_TC = 2;
			tempTC = temp;
            break;
        // enemigo inminente direccion b
        case 11:
            com = MAX_B;
			memo_TC = 10;
			tempTC = temp;
            break;
        // enemigo detectado en direccion b izquierda
        case 12:
            com = ATAQUE_BI;
			memo_TC = 7;
			tempTC = temp;
            break;
        //incoherencia, posible falla de los sensores en direccion b(accion por definir que es mejor)
        case 13:
            com = MAX_B;
			memo_TC = 10;
			tempTC = temp;
            break;
        // enemigo detectado en direccion b derecha
        case 14:
            com = ATAQUE_BD;
			memo_TC = 8;
			tempTC = temp;
            break;
        // enemigo detectado muy a la izquierda en direccion b
        case 15:
            com = PRO_BI;
			memo_TC = 3;
			tempTC = temp;
            break;
        // enemigo detectado lejos en direccion b
        case 16:
            com = DIR_B;
			memo_TC = 10;
			tempTC = temp;
            break;
        // enemigo detectado muy a la derecha en direccion b
        case 17:
            com = PRO_BD;
			memo_TC = 4;
			tempTC = temp;
            break;
		//memora corto plazo del sector AI
		case 18:
			com = PRO_AI;
			memo_TL = 1;
			tempTL = temp;
			break;
		//memoria corto plazo del sector AD
		case 19:
			com = PRO_AD;
			memo_TL = 2;
			tempTL = temp;
			break;
		//memoria corto plazo del sector BI
		case 20:
			com = PRO_BI;
			memo_TL = 3;
			tempTL = temp;
			break;
		//memoria corto plazo del sector BD
		case 21:
			com = PRO_BD;
			memo_TL = 4;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector AI
		case 22:
			com = ATAQUE_AI;
			memo_TL = 1;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector AD
		case 23:
			com = ATAQUE_AD;
			memo_TL = 2;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector BI
		case 24:
			com = ATAQUE_BI;
			memo_TL = 3;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector BD
		case 25:
			com = ATAQUE_BD;
			memo_TL = 4;
			tempTL = temp;
			break;
		//memoria corto plazo direccion A
		case 26:
			com = MAX_A;
			memo_TL = 0;
			break;
		//memoria corto plazo DIreccion B
		case 27:
			com = MAX_B;
			memo_TL = 0;
			break;
		//memoria largo plazo del sector AI
		case 28:
			com = PRO_AI;
			break;
		//memoria largo plazo del sector AD
		case 29:
			com = PRO_AD;
			break;
		//memoria largo plazo del sector BI
		case 30:
			com = PRO_BI;
			break;
		//memoria largo plazo del sector BD
		case 31:
			com = PRO_BD;
			break;
		//avanze de estrella
		case 32:
			//logica para que avanze formando la estrella
			if (temp - tempE1 >= tiempo4) {
				com = GIRO;
    			memo_E = false;        // ir a girar
    			tempE2 = temp;         // inicia tiempo del giro
			}else{
      			com = DIR_A;
			}
      		break;
		//giro de estrella
		case 33:
    		if(temp - tempE2 >= tiempo5){
				com = DIR_A;
    		    memo_E = true;      // Termina giro → pasará a estado 8
    		    tempE1 = temp;      // Reiniciar timer del avance
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
			memo_TC = 9;
			tempTC = temp;
            break;
        //enemigo detectado en direccion a izquierda
        case 5:
            com = ATAQUE_AI;
			memo_TC = 5;
			tempTC = temp;
			break;
        //incoherencia, posible falla de los sensores en direccion a(accion por definir que es mejor)
        case 6:
            com = MAX_A;
			memo_TC = 9;
			tempTC = temp;
            break;
        // enemigo detectado ne direccion a derecha
        case 7:
            com = ATAQUE_AD;
			memo_TC = 6;
			tempTC = temp;
            break;
        // enemigo detectado muy a la izquierda en direccion a
        case 8:
            com = PRO_AI;
			memo_TC = 1;
			tempTC = temp;
			break;
        //enemigo detectado lejos en direccion a
        case 9:
            com = DIR_A;
			memo_TC = 9;
			tempTC = temp;
            break;
        // enemigo detectado muy a la derecha en direccion a
        case 10:
            com = PRO_AD;
			memo_TC = 2;
			tempTC = temp;
            break;
        // enemigo inminente direccion b
        case 11:
            com = MAX_B;
			memo_TC = 10;
			tempTC = temp;
            break;
        // enemigo detectado en direccion b izquierda
        case 12:
            com = ATAQUE_BI;
			memo_TC = 7;
			tempTC = temp;
            break;
        //incoherencia, posible falla de los sensores en direccion b(accion por definir que es mejor)
        case 13:
            com = MAX_B;
			memo_TC = 10;
			tempTC = temp;
            break;
        // enemigo detectado en direccion b derecha
        case 14:
            com = ATAQUE_BD;
			memo_TC = 8;
			tempTC = temp;
            break;
        // enemigo detectado muy a la izquierda en direccion b
        case 15:
            com = PRO_BI;
			memo_TC = 3;
			tempTC = temp;
            break;
        // enemigo detectado lejos en direccion b
        case 16:
            com = DIR_B;
			memo_TC = 10;
			tempTC = temp;
            break;
        // enemigo detectado muy a la derecha en direccion b
        case 17:
            com = PRO_BD;
			memo_TC = 4;
			tempTC = temp;
            break;
		//memoria corto plazo del sector AI
		case 18:
			com = PRO_AI;
			memo_TL = 1;
			tempTL = temp;
			break;
		//memoria corto plazo del sector AD
		case 19:
			com = PRO_AD;
			memo_TL = 2;
			tempTL = temp;
			break;
		//memoria corto plazo del sector BI
		case 20:
			com = PRO_BI;
			memo_TL = 3;
			tempTL = temp;
			break;
		//memoria corto plazo del sector BD
		case 21:
			com = PRO_BD;
			memo_TL = 4;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector AI
		case 22:
			com = ATAQUE_AI;
			memo_TL = 1;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector AD
		case 23:
			com = ATAQUE_AD;
			memo_TL = 2;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector BI
		case 24:
			com = ATAQUE_BI;
			memo_TL = 3;
			tempTL = temp;
			break;
		//memoria corto plazo suave del sector BD
		case 25:
			com = ATAQUE_BD;
			memo_TL = 4;
			tempTL = temp;
			break;
		//memoria corto plazo de frente
		case 26:
			com = MAX_A;
			memo_TL = 0;
			break;
		//memoria corto plazo hacia atras
		case 27:
			com = MAX_B;
			memo_TL = 0;
			break;
		//memoria del sector AI
		case 28:
			com = PRO_AD;
			break;
		//memoria del sector AD
		case 29:
			com = PRO_AI;
			break;
		//memoria del sector BI
		case 30:
			com = PRO_BD;
			break;
		//memoria del sector BD
		case 31:
			com = PRO_BI;
			break;
		//avanze de estrella
		case 32:
			//logica para que avanze formando la estrella
			if (temp - tempE1 >= tiempo4) {
				com = GIRO;
    			memo_E = false;        // ir a girar
    			tempE2 = temp;         // inicia tiempo del giro
			}else{
      			com = DIR_A;
			}
      		break;
		//giro de estrella
		case 33:
    		if(temp - tempE2 >= tiempo5){
				com = DIR_A;
    		    memo_E = true;      // Termina giro → pasará a estado 8
    		    tempE1 = temp;      // Reiniciar timer del avance
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
	tiempo1 = nvs.leer("retroceso",tiempo1);
	tiempo2 = nvs.leer("tof_corto_plazo",tiempo2);
	tiempo3 = nvs.leer("tof_largo_plazo",tiempo3);
	tiempo4 = nvs.leer("recta_star",tiempo4);
	tiempo5 = nvs.leer("giro_star",tiempo5);
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
