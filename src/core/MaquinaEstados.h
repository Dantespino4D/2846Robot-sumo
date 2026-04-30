#ifndef MAQUINAESTADOS_H
#define MAQUINAESTADOS_H

// comandos de movimiento
#include <cstdint>
#define ALTO 0
#define DIR_A 1
#define DIR_B 2
#define EVA_A 3
#define EVA_B 4
#define ATAQUE_AI 5
#define ATAQUE_BI 6
#define ATAQUE_AD 7
#define ATAQUE_BD 8
#define PRO_AI 9
#define PRO_BI 10
#define PRO_AD 11
#define PRO_BD 12
#define MAX_A 13
#define MAX_B 14
#define GIRO 15
#define HUIR_A 16
#define HUIR_B 17

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

class EstrategiaBase;
class EstrategiaPrototipo;
class Estrategia1;
class Estrategia2;

class MaquinaEstados{
	friend class EstrategiaPrototipo;
	friend class EstrategiaEstandar;
	friend class Estrategia1;
	friend class Estrategia2;
	private:
		// variables que cuentan el tiempo
		unsigned long tempTC;
		unsigned long tempTL;
		unsigned long tempC;
		unsigned long tempE1;
		unsigned long tempE2;
		unsigned long tempS;
		unsigned long tempEva;

		//tiempos establecidos
		int tiempo1;
		int tiempo2;
		int tiempo3;
		int tiempo4;
		int tiempo5;
		int tiempo6;

	public:
		float umbral_stall;
		unsigned long tiempo_stall;
		float corrienteA;

	private:
		// handle de la tarea de los motores
		TaskHandle_t* motr;

		//variables de control
		volatile int modo;
		volatile int estrategia;
		volatile int ciclo;
		int ini;

		bool stall;
		int memo_eva;

		int memo_TC;//control de los ToF a corto plazo
				   //0 = sin memoria, 1 = AI pro, 2 = AD pro, 3 = BI pro, 4 = BD pro, 5 = AI suave, 6 = AD suave, 7 = BI suave, 8 = BD suave, 9 = frente, 10 = atras

		int memo_TL;//control de los ToF a largo plazo
			   //0 = sin memoria, 1 = AI, 2 = AD, 3 = BI, 4 = BD

		int memo_C;//control de los sc
					//0 = sin memoria, 1 = sc_1, 2 = sc_2

		bool memo_E;//contol estrella

		// Instancias de estrategias
		EstrategiaPrototipo* estPrototipo;
		Estrategia1* estE1;
		Estrategia2* estE2;

		//puntero de la estrategia actual
		EstrategiaBase* estActual;

		// version del hardware
		bool final;

		//metodo de validacion del tiempo
		void tiempo();

		//lee los datos de la nvs
		void nvsLeer();

		//metodo para detectar atascos
		bool detectarStall(float corrienteA);
	public:
		//constructor
		MaquinaEstados(int _tiempo1, int _tiempo2, int _tiempo3, int tiempo4, int _tiempo5, TaskHandle_t* _motr, bool _final);

		//destructor
		~MaquinaEstados();

		//bloqueo de copia
		MaquinaEstados(const MaquinaEstados&) = delete;
		MaquinaEstados& operator=(const MaquinaEstados&) = delete;

		//maquina de estados
		void logica();

		//se obtiene el ciclo
		void cicloR(int c, int i);

		//metodo que entrega el estado actual
		void datos(int* _modo, int* _estra, int* _ciclo, int* _ini, bool* _stall);
};
#endif // !DEBUG
