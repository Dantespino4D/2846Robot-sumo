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

	//inicia el timer
	LL_TIM_EnableCounter(TIM1);

	LL_TIM_EnableAllOutputs(TIM1);
}

void Motores::ejecutarAccion(uint16_t* rampa){
	LL_TIM_DisableCounter(TIM6);
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

	LL_TIM_ConfigDMABurst(TIM1, LL_TIM_DMABURST_BASEADDR_CCR1, LL_TIM_DMABURST_LENGTH_4TRANSFERS);

	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)rampa);
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&TIM1->DMAR);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, PASOS_RAM*4);

	LL_TIM_EnableDMAReq_UPDATE(TIM6);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH4);
	LL_TIM_EnableCounter(TIM1);
	LL_TIM_EnableAllOutputs(TIM1);

	LL_TIM_SetCounter(TIM6, 0);
	LL_TIM_EnableCounter(TIM6);
}

Motores mot;

extern "C" void gatillo(uint16_t* rampa){
	mot.ejecutarAccion(rampa);
}
