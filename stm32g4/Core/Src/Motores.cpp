#include "Motores.h"
#include "protocolo/sumo_protocol.h"
#include "stm32g474xx.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_tim.h"
#include <cstdlib>

Motores::Motores(){}

//se inicializa todo
void Motores::begin(){
	//forzar freno al iniciar
	TIM1->CCR1 = 1023;
	TIM1->CCR2 = 1023;
	TIM1->CCR3 = 1023;
	TIM1->CCR4 = 1023;

	//habilitar los canales del timer
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH4);

	//inicia el TIM1
	LL_TIM_EnableCounter(TIM1);
	LL_TIM_EnableAllOutputs(TIM1);

	//activacion del freno BRK
	LL_TIM_EnableIT_BRK(TIM1);

	//limpiar las banderas de interrupcion
	LL_TIM_ClearFlag_UPDATE(TIM6);

	//se inicia el TIM6
	LL_TIM_EnableIT_UPDATE(TIM6);
	LL_TIM_EnableCounter(TIM6);
}
