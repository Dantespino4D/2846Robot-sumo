#include "MaquinaEstados.h"
#include "Nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include <cstdint>

MaquinaEstados::MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int _tiempo4, TaskHandle_t* _motr):
	temp1(0),
	temp2(0),
	temp3(0),
	temp4(0),
	temp5(0),
	temp6(0),
	tiempo1(_tiempo1),
	tiempo2(_tiempo2),
	tiempo3(_tiempo3),
	tiempo4(_tiempo4),
	motr(_motr),
	modo(6),
	estrategia(0),
	ciclo(0),
	ini(0),
	memo1(false),
	memo2(false),
	memo3(false),
	memo4(false),
	memo5(false),
	memo6(false),
	memo7(false)
{
	nvsLeer();
}

//se cuentan kis tiempos
void MaquinaEstados::tiempo(){
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	// condiciones que evaluan si ya pasaron los tiempos
    if (temp - temp1 >= tiempo1) {
		//tiempo de ataque ciego dir a derecha
    	memo1 = false;
		//tiempo de ataque ciego dir b izquierda
		memo6 = false;
    }
    if (temp - temp2 >= tiempo1) {
		//tiempo de ataque ciego dir b derecha
      	memo2 = false;
		//tiempo de ataque ciego dir b izquierda
		memo7 = false;
    }
    if (temp - temp3 >= tiempo2) {
		//timepo de retroceso dir a
    	memo3 = false;
    }
    if (temp - temp4 >= tiempo2) {
		//tiempo de retroceso dir b
     	memo4 = false;
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
	else if (memo3) {
  		modo = 2;
	}
	// si deja de detectar el limite por sc 2
	else if (memo4) {
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
	else if (memo1) {
  		modo = 6;
	}
	// si deja de detectar al robot por ojos 2
	else if (memo2) {
		modo = 7;
    }
    // si no detecta nada
	else if(memo5){
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
	else if (memo3) {
		modo = 2;
	}
	// si deja de detectar el limite por sc 2
	else if (memo4) {
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
	else if (memo5) {
	  		modo = 18;
	}
	else if(memo1) {
	  		modo = 19;
	}
	else if (memo2) {
		modo = 20;
	}
	else if (memo6) {
	  		modo = 21;
	}
	else if (memo7) {
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
    int com;

	//se guarda el tiempo
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

    // ejecuta el estado
    switch (modo) {
		// detiene el movimiento y retrocede en direccion b
    	case 0:
      		com = DIR_B;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		memo3 = true;
      		temp3 = temp;
      		break;
      	// detiene el movimiento y retrocede en direccion a
    	case 1:
    	  	com = DIR_A;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		memo4 = true;
      		temp4 = temp;
      		break;
		// continua avanzando en direccion b por un tiempo definido para alejarse del borde
    	case 2:
      		com = DIR_B;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		break;
    	// continua avanzando en direccion a por un tiempo definido para alejarse del borde
    	case 3:
      		com = DIR_A;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		break;
    	// avanza en direccion a
    	case 4:
      		com = ATAQUE_AI;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		memo1 = true;
      		temp1 = temp;
      		break;
    	// avanza en direccion b
    	case 5:
      		com = ATAQUE_BI;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		memo2 = true;
      		temp2 = temp;
      		break;
    	// avanza por un tiempo definido de 4 segundo en direccion a
    	case 6:
      		com = ATAQUE_AI;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		break;
    	// avanza por un tiempo definido de 4 segundos en direccion b
    	case 7:
      		com = ATAQUE_BI;
        	xTaskNotify(*motr, com, eSetValueWithOverwrite);
      		break;
      	// da vueltas hasta encontrar el robot
    	case 8:
      		//aqui pondre la logica para que avanze formando la estrella
			if (temp - temp5 >= tiempo3) {
    			memo5 = false;        // ir a girar
    			temp6 = temp;         // inicia tiempo del giro
			}else{
      			com = DIR_A;
        		xTaskNotify(*motr, com, eSetValueWithOverwrite);
			}
      		break;
		case 9:
    		if(temp - temp6 >= tiempo4){
    		    memo5 = true;      // Termina giro → pasará a estado 8
    		    temp5 = temp;      // Reiniciar timer del avance
    		} else {
    		    com = GIRO;
        		xTaskNotify(*motr, com, eSetValueWithOverwrite);
    		}
            break;
	}
}

//ejecucion de la estrategia 1
void MaquinaEstados::ejecucionE1(){
	//variable del comandos
	int com;

	//variable que guarda el tiempo
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	switch (modo) {
		// detiene el movimiento y retrocede en direccion b
		case 0:
	  		com = DIR_B;
	  		memo3 = true;
	  		temp3 = temp;
	  		break;
	  	// detiene el movimiento y retrocede en direccion a
		case 1:
		  	com = DIR_A;
	  		memo4 = true;
	  		temp4 = temp;
	  		break;
		// continua avanzando en direccion b por un tiempo definido para alejarse del borde
		case 2:
	  		com = DIR_B;
	  		break;
		// continua avanzando en direccion a por un tiempo definido para alejarse del borde
		case 3:
	  		com = DIR_A;
	  		break;
		// enemigo inmnente direccion b
		case 4:
	  		com = MAX_B;
	  		break;
		//enemigo a detectado a mediana distancia direccion b izquierda
		case 5:
			com = MED_BI;

		//incoherencia, posible falla de los sensores en direccion b
		case 6:
			com = MAX_B;
			break;
	  	// da vueltas hasta encontrar el robot
		case 8:
	  		com = GIRO;
	  		break;
		// enemigo inminente direccio a
		case 11:
			com = MAX_A;
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

	//seleccion de estado
	seleccionP();

	//se ejecuta dependiendo de la estrategia
	switch (estrategia) {
		case 1:
			ejecucionEP();
			break;
		case 2:
			ejecucionE1();
			break;
		case 3:
			ejecucionE2();
			break;

	}
}

void MaquinaEstados::ejecucionE2(){

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
