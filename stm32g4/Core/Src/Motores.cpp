#include "Motores.h"
#include "protocolo/sumo_protocol.h"
#include "stm32g474xx.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_tim.h"
#include <cstdlib>

Motores::Motores(){}

//se inicializa todo
void Motores::begin(){
	//enserde los canales del timer
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH4);

	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&TIM1->DMAR);

	//inicia el timer
	LL_TIM_EnableCounter(TIM1);

	LL_TIM_EnableAllOutputs(TIM1);

	//se inicia el TIM3
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableIT_UPDATE(TIM3);
}
