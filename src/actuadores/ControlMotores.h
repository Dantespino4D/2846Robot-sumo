#ifndef CONTROLMOTORES_H
#define CONTROLMOTORES_H

#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include <cstdint>

class ControlMotores{
	private:
		// MCPWM handles
		mcpwm_timer_handle_t timer;
		mcpwm_oper_handle_t oper_izq;
		mcpwm_oper_handle_t oper_der;
		mcpwm_cmpr_handle_t cmpr_izq;
		mcpwm_cmpr_handle_t cmpr_der;
		mcpwm_gen_handle_t gen_izq_a;
		mcpwm_gen_handle_t gen_izq_b;
		mcpwm_gen_handle_t gen_der_a;
		mcpwm_gen_handle_t gen_der_b;

		//variables necesarias
		uint32_t freq;
		uint16_t period;

		gpio_num_t mot2[2];
		gpio_num_t mot[2];

		//variables de la rampa
		int tRam;

		//funcion velocidad
		void velocidad(int16_t vel_1, int16_t vel_2, bool ram);

		//variables que definien las velocidades
		int16_t vel_nI;
		int16_t vel_nD;
		int16_t vel_aI;
		int16_t vel_aD;
		int16_t vel_mI;
		int16_t vel_mD;
		int16_t vel_pI;
		int16_t vel_pD;
		int16_t vel_gI;
		int16_t vel_gD;
		int16_t vel_eI; // Evasión Izquierda
		int16_t vel_eD; // Evasión Derecha

		//variables que guardan la velocidad actual
		volatile int16_t vel1;
		volatile int16_t vel2;

		//booleano de la rampa
		volatile bool rampa;

		//variables de la velocidad objetivo
		volatile int16_t vel1_obj;
		volatile int16_t vel2_obj;

		//variables de la tarea estatica
		StaticTask_t tcbMotores;
		StackType_t stackMotores[2048];

		//metodos de acciones
		void dir_a();
		void dir_b();
		void ataque_ai();
		void ataque_bi();
		void ataque_ad();
		void ataque_bd();
		void max_a();
		void max_b();
		void pronunciado_ai();
		void pronunciado_bi();
		void pronunciado_ad();
		void pronunciado_bd();
		void giro();
		void evasion_a();
		void evasion_b();

		//metodo para leer y aplicar los valores de la nvs
		void nvsLeer();
		//tarea de rampa
		static void tareaRampa(void* arg);
	public:
		//variables control e inicializacion
		ControlMotores(gpio_num_t motA2, gpio_num_t motB2, gpio_num_t motA1, gpio_num_t motB1);
		//begin
		void begin();
		//controlador de acciones
		void controlador(int accion);
		//alto
		void alto();
		//envia los datos a telemetria
		void velocidades(int16_t* v1, int16_t* v2, int16_t* vo1, int16_t* vo2);
};

#endif
