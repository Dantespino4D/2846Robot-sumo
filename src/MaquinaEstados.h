#ifndef MAQUINAESTADOS_H
#define MAQUINAESTADOS_H

// comandos de movimiento
#define ALTO 0
#define DIR_A 1
#define DIR_B 2
#define GIRO 3

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

		//tiempos establecidos
		int tiempo1;
		int tiempo2;

		// alerta del limite
		SemaphoreHandle_t& alerta;
		SemaphoreHandle_t& alerta2;

		// alerta de deteccion del rival
		SemaphoreHandle_t& enemigo;
		SemaphoreHandle_t& enemigo2;

		// orden de una accion
		QueueHandle_t &orden;

		//variables de control
		int modo;
		bool memo1;
		bool memo2;
		bool memo3;
		bool memo4;

		//metodo de validacion del tiempo
		void tiempo();

		//seleccion de estado
		void seleccion();

		//sejecuta el estado
		void ejecucion();
	public:
		//constructor
		MaquinaEstados(int _tiempo1, int _tiempo2,SemaphoreHandle_t& _alerta, SemaphoreHandle_t& _alerta2,
                   SemaphoreHandle_t& _enemigo, SemaphoreHandle_t& _enemigo2,
            		QueueHandle_t& _orden);

		//maquina de estados
		void logica();
};
#endif // !DEBUG
