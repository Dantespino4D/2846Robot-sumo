#include "Motores.h"

Motores::Motores(TIM_HandleTypeDef *htim):
	htim1(htim),

	//velocidades por defecto
	vel_nI(700),
	vel_nD(700),
	vel_aI(950),
	vel_aD(800),
	vel_mI(1023),
	vel_mD(1023),
	vel_pI(950),
	vel_pD(600),
	vel_gI(950),
	vel_gD(-800),
	vel_hI(950),
	vel_hD(950)
{}

//se inicializa todo
void Motores::begin(){
	//inicia el timer para generar la señal PWM
	HAL_TIM_PWM_Start(htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(htim1, TIM_CHANNEL_4);
}

//estavlece las velocidades
void Motores::velocidad(int16_t vel_1, int16_t vel_2, bool ram){
	//detecta si se usara rampa
	if(ram){
		//futura rampa de aceleracion con CORDIC
	}

	//se establecen las velocidades de los motores de la izquierda
	if(vel_1 < 0){
		//si es negativa
		vel_1 = -vel_1;
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_2, vel_1);
	}else if(vel_1 > 0){
		//si es positiva
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_1, vel_1);
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_2, 0);
	}else{
		//freno
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_1, 1023);
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_2, 1023);
	}

	//se establecen las velocidades de los motores de la derecha
	if(vel_2 < 0){
		//si es negativa
		vel_2 = -vel_2;
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_4, vel_2);
	}else if(vel_2 > 0){
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_3, vel_2);
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_4, 0);
	}else{
		//freno
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_3, 1023);
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_4, 1023);
	}
}
