#include "MaquinaEstados.h"
#include "Nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <cstdint>

#include "Estrategia1.h"
#include "Estrategia2.h"

MaquinaEstados::MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int _tiempo4, int _tiempo5, TaskHandle_t* _motr):
	tempTL(0),
	tempC(0),
	tempE1(0),
	tempE2(0),
	tempS(0),
	tempEva(0),
	tiempo1(_tiempo1),
	tiempo2(_tiempo2),
	tiempo3(_tiempo3),
	tiempo4(_tiempo4),
	tiempo5(_tiempo5),
	tiempo6(1000),
	umbral_stall(2.0f),
	tiempo_stall(250),
	corrienteA(0.0f),
	motr(_motr),
	modo(-1),
	estrategia(0),
	ciclo(0),
	ini(0),
	stall(false),
	memo_eva(0),
	memo_TC(0),
	memo_TL(0),
	memo_C(0),
	memo_E(false)
{
	estE1 = new Estrategia1();
	estE2 = new Estrategia2();

	nvsLeer();
	switch(estrategia){
		case 0:
			estActual = estE1;
			break;
		case 1:
			estActual = estE2;
			break;
		default:
			estActual = estE1;
			break;
	}
}

MaquinaEstados::~MaquinaEstados() {
	delete estE1;
	delete estE2;
}

// se cuentan kis tiempos
void MaquinaEstados::tiempo(){
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

	// condiciones que evaluan si ya pasaron los tiempos
    if (temp - tempC >= (unsigned long)tiempo1) {
		//timepo de retroceso dir a
    	memo_C = 0;
    }
	if (temp - tempTC >= (unsigned long)tiempo2) {
		//tiempo de memoria a corto plazo
		memo_TC = 0;
	}
    if (temp - tempTL >= (unsigned long)tiempo3) {
		//tiempo de la memoria a largo plazo
    	memo_TL = 0;
    }
	if (stall && (temp - tempEva >= (unsigned long)tiempo6)) {
		//tiempo de la evasion
		stall = false;
		memo_eva = 0;
	}
}

void MaquinaEstados::logica(){
	tiempo();
	if(estActual != nullptr){
		estActual->seleccion(this);
		estActual->ejecucion(this);
	}
}

void MaquinaEstados::nvsLeer(){
	Nvs nvs("tiempos");
	tiempo1 = nvs.leer("retroceso",tiempo1);
	tiempo2 = nvs.leer("tof_corto_plazo",tiempo2);
	tiempo3 = nvs.leer("tof_largo_plazo",tiempo3);
	tiempo4 = nvs.leer("recta_star",tiempo4);
	tiempo5 = nvs.leer("giro_star",tiempo5);
	tiempo_stall = nvs.leer("t_stall", tiempo_stall);
	tiempo6 = nvs.leer("evasion", tiempo6);
	estrategia = nvs.leer("estrategia",estrategia);

	Nvs mot("motores");
	umbral_stall = mot.leerFloat("u_stall", umbral_stall);
}

//metodo para detectar atascos
bool MaquinaEstados::detectarStall(float corrienteA){
	unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);
	bool mov_frontal = (modo >= 6 && modo <= 12) || modo == 24 || modo == 25 || modo == 28 || modo == 30 || modo == 31;
	bool mov_trasero = (modo >= 13 && modo <= 19) || modo == 26 || modo == 27 || modo == 29 || modo == 32 || modo == 33;

	if (mov_frontal || mov_trasero) {
		if (corrienteA > umbral_stall) {
			if (tempS == 0) tempS = temp;
			if (temp - tempS > (unsigned long)tiempo_stall) {
				memo_eva = mov_frontal ? 1 : 2;
				tempS = 0;
				return true;
			}
		} else {
			tempS = 0;
		}
	} else {
		tempS = 0;
	}
	return false;
}

//metodo que recibe la duracion del ciclo
void MaquinaEstados::cicloR(int c, int i){
	ciclo = c;
	ini = i;
}

//metodo que entrega el estado actual
void MaquinaEstados::datos(int* _modo, int* _estra, int* _ciclo, int* _ini, bool* _stall){
	 *_modo = modo;
	 *_estra = estrategia;
	 *_ciclo =ciclo;
	 *_ini = ini;
	 *_stall = stall;
}
