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

//para el robot
void Motores::alto(){
	velocidad(0, 0, false);
}

//avanzar en direccion a
void Motores::dir_a(){
	velocidad(vel_nI, vel_nD, true);
}

//avanzar en direccion b
void Motores::dir_b(){
	velocidad(-vel_nI, -vel_nD, true);
}

//atacar en direccion a izquierda
void Motores::ataque_ai(){
	velocidad(vel_aI, vel_aD, true);
}

//atacar en direccion b izquierda
void Motores::ataque_bi(){
	velocidad(-vel_aI, -vel_aD, true);
}

//atacar en direccion a derecha
void Motores::ataque_ad(){
	velocidad(vel_aD, vel_aI, true);
}

//atacar en direccion b derecha
void Motores::ataque_bd(){
	velocidad(-vel_aD, -vel_aI, true);
}

//maxima velocidad en direccion a
void Motores::max_a(){
	velocidad(vel_mI, vel_mD, true);
}

//maxima velocidad en direccion b
void Motores::max_b(){
	velocidad(-vel_mI, -vel_mD, true);
}

//ataca de forma pronunciada en direccion a izquierda
void Motores::pronunciado_ai(){
	velocidad(vel_pI, vel_pD, true);
}

//ataca de forma pronunciada en direccion b izquierda
void Motores::pronunciado_bi(){
	velocidad(-vel_pI, -vel_pD, true);
}

//ataca de forma pronunciada en direccion a derecha
void Motores::pronunciado_ad(){
	velocidad(vel_pD, vel_pI, true);
}

//ataca de forma pronunciada en direccion b derecha
void Motores::pronunciado_bd(){
	velocidad(-vel_pD, -vel_pI, true);
}

//giro de busqueda
void Motores::giro(){
	velocidad(vel_gI, vel_gD, false);
}

//maniobra de evacion en direccion a
void Motores::evacion_a(){
	velocidad(vel_eI, vel_eD, false);
}

//maniobra de evacion en direccion b
void Motores::evacion_b(){
	velocidad(-vel_eI, -vel_eD, false);
}

//maniobra de huida del borde en direccion a
void Motores::huir_a(){
	velocidad(vel_hI, vel_hD, false);
}

//maniobra de huida del borde en direccion b
void Motores::huir_b(){
	velocidad(-vel_hI, -vel_hD, false);
}

//eleccion de movimieno
void Motores::controlador(int accion){
	switch(accion){
		case 0:
			alto();
			break;
		case 1:
			dir_a();
			break;
		case 2:
			dir_b();
			break;
		case 3:
			ataque_ai();
			break;
		case 4:
			ataque_bi();
			break;
		case 5:
			ataque_ad();
			break;
		case 6:
			ataque_bd();
			break;
		case 7:
			max_a();
			break;
		case 8:
			max_b();
			break;
		case 9:
			pronunciado_ai();
			break;
		case 10:
			pronunciado_bi();
			break;
		case 11:
			pronunciado_ad();
			break;
		case 12:
			pronunciado_bd();
			break;
		case 13:
			giro();
			break;
		case 14:
			evacion_a();
			break;
		case 15:
			evacion_b();
			break;
		case 16:
			huir_a();
			break;
		case 17:
			huir_b();
			break;
		default:
			alto();
	}
}
