#ifndef MAQUINAESTADOS_H
#define MAQUINAESTADOS_H

// comandos de movimiento
#include <cstdint>
#define ALTO 0
#define DIR_A 1
#define DIR_B 2
#define ATAQUE_A 3
#define ATAQUE_B 4
#define GIRO 5

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

class MaquinaEstados{
	private:
		// variables que cuentan el tiempo
		unsigned long temp1;
		unsigned long temp2;
		unsigned long temp3;
		unsigned long temp4;
		unsigned long temp5;
		unsigned long temp6;

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

		bool memo1;
		bool memo2;
		bool memo3;
		bool memo4;
		bool memo5;

		//metodo de validacion del tiempo
		void tiempo();

		//seleccion de estado
		void seleccion();

		//sejecuta el estado
		void ejecucion();

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
