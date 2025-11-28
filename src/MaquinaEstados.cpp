#include "MaquinaEstados.h"
#include "freertos/task.h"

MaquinaEstados::MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int _tiempo4, SemaphoreHandle_t& _alerta, SemaphoreHandle_t& _alerta2,
                   SemaphoreHandle_t& _enemigo, SemaphoreHandle_t& _enemigo2,
                   QueueHandle_t& _orden):
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
	alerta(_alerta),
	alerta2(_alerta2),
	enemigo(_enemigo),
	enemigo2(_enemigo2),
	orden(_orden),
	modo(6),
	memo1(false),
	memo2(false),
	memo3(false),
	memo4(false),
	memo5(false)
{}

//se cuentan kis tiempos
void MaquinaEstados::tiempo(){
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	// condiciones que evaluan si ya pasaron los tiempos
    if (temp - temp1 >= tiempo1) {
    	memo1 = false;
    }
    if (temp - temp2 >= tiempo1) {
      	memo2 = false;
    }
    if (temp - temp3 >= tiempo2) {
    	memo3 = false;
    }
    if (temp - temp4 >= tiempo2) {
     	memo4 = false;
    }
}

// selecciona el estado
void MaquinaEstados::seleccion(){
	// si detecta el limite por sc_1
	if (xSemaphoreTake(alerta, 0) == pdTRUE) {
  		modo = 0;
	}
	// si detecta el limite por sc_2
	else if (xSemaphoreTake(alerta2, 0) == pdTRUE) {
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
	else if (xSemaphoreTake(enemigo, 0) == pdTRUE) {
  		modo = 4;
	}
    // si detecta el robot por ojos 2
	else if (xSemaphoreTake(enemigo2, 0) == pdTRUE) {
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

void MaquinaEstados::ejecucion(){
	// variable del comandos
    int com;

	//se guarda el tiempo
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

    // ejecuta el estado
    switch (modo) {
		// detiene el movimiento y retrocede en direccion b
    	case 0:
      		com = DIR_B;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		memo3 = true;
      		temp3 = temp;
      		break;
      	// detiene el movimiento y retrocede en direccion a
    	case 1:
    	  	com = DIR_A;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		memo4 = true;
      		temp4 = temp;
      		break;
		// continua avanzando en direccion b por un tiempo definido para alejarse del borde
    	case 2:
      		com = DIR_B;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		break;
    	// continua avanzando en direccion a por un tiempo definido para alejarse del borde
    	case 3:
      		com = DIR_A;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		break;
    	// avanza en direccion a
    	case 4:
      		com = ATAQUE_A;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		memo1 = true;
      		temp1 = temp;
      		break;
    	// avanza en direccion b
    	case 5:
      		com = ATAQUE_B;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		memo2 = true;
      		temp2 = temp;
      		break;
    	// avanza por un tiempo definido de 4 segundo en direccion a
    	case 6:
      		com = ATAQUE_A;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		break;
    	// avanza por un tiempo definido de 4 segundos en direccion b
    	case 7:
      		com = ATAQUE_B;
      		xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
      		break;
      	// da vueltas hasta encontrar el robot
    	case 8:
      		//aqui pondre la logica para que avanze formando la estrella
			if (temp - temp5 >= tiempo3) {
    			memo5 = false;        // ir a girar
    			temp6 = temp;         // inicia tiempo del giro
			}else{
      			com = DIR_A;
      			xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
			}
      		break;
		case 9:
    		if(temp - temp6 >= tiempo4){
    		    memo5 = true;      // Termina giro → pasará a estado 8
    		    temp5 = temp;      // Reiniciar timer del avance
    		} else {
    		    com = GIRO;
    		    xQueueSend(orden, &com, 10 / portTICK_PERIOD_MS);
    		}
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
