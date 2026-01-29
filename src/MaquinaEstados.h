#ifndef MAQUINAESTADOS_H
#define MAQUINAESTADOS_H

// comandos de movimiento
#include <cstdint>
#define ALTO 0
#define DIR_A 1
#define DIR_B 2
#define ATAQUE_AI 3
#define ATAQUE_BI 4
#define ATAQUE_AD 5
#define ATAQUE_BD 6
#define PRO_AI 7
#define PRO_BI 8
#define PRO_AD 9
#define PRO_BD 10
#define MAX_A 11
#define MAX_B 12
#define GIRO 13

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

class MaquinaEstados{
	private:
		// variables que cuentan el tiempo
		unsigned long tempT;
		unsigned long tempC;
		unsigned long tempE1;
		unsigned long tempE2;

		//tiempos establecidos
		int tiempo1;
		int tiempo2;
		int tiempo3;
		int tiempo4;

		// handle de la tarea de los motores
		TaskHandle_t* motr;

		//variables de control
		int modo;
		int estrategia;
		int ciclo;
		int ini;

		int memo_T;//control de los ToF
				   //0 = sin memoria, 1 = AI, 2 = AD, 3 = BI, 4 = BD

		int memo_C;//control de los sc
					//0 = sin memoria, 1 = sc_1, 2 = sc_2

		bool memo_E;//contol estrella


		//metodo de validacion del tiempo
		void tiempo();

		//seleccion de estado(prototipo)
		void seleccionP();

		//seleccion de estado(final)
		void seleccion();

		//se ejecuta el estado(prototipo)
		void ejecucionEP();

		//se ejecuta el estado(estrategia 1)
		void ejecucionE1();

		//se ejecutara el estado(estrategia 2)
		void ejecucionE2();

		//lee los datos de la nvs
		void nvsLeer();
	public:
		//constructor
		MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int tiempo4, TaskHandle_t* _motr);

		//maquina de estados
		void logica();

		//se obtiene el ciclo
		void cicloR(int c, int i);

		//metodo que entrega el estado actual
		void datos(int* _modo, int* _estra, int* _ciclo, int* _ini);
};
#endif // !DEBUG
