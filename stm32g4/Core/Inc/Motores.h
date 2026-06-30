#ifndef MOTORES_H
#define MOTORES_H

#include "stm32g4xx_hal.h"
#include <cstdint>

class Motores {
	public:
		Motores(TIM_HandleTypeDef* htim1);
		void begin();
		void velocidad(int16_t vel_1, int16_t vel_2, bool ram);

	private:
		TIM_HandleTypeDef* htim1;

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
		int16_t vel_eI;
		int16_t vel_eD;
		int16_t vel_hI;
		int16_t vel_hD;

		//metodos de acciones
		void alto();
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
		void evacion_a();
		void evacion_b();
		void huir_a();
		void huir_b();

		//metodo de eleccion de movimiento
		void controlador(int accion);
};

#endif // MOTORES_H
