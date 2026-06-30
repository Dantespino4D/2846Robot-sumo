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


