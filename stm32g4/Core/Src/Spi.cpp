#include "stm32g474xx.h"
#include "Spi.h"
#include "protocolo/sumo_protocol.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_crc.h"
#include "stm32g4xx_ll_dac.h"
#include <cstddef>
#include <cstdint>

extern "C" void gatillo(uint16_t* rampa);
int16_t velObjetivo_1 = 0;
int16_t velObjetivo_2 = 0;

Spi::Spi() :
	bufferA{},
	bufferB{},
	cont(0),
	bufferCpu(bufferB),
	bufferDma(bufferA)
{}

void Spi::begin() {
	recibir();
}

//se calcula el checksum
uint8_t Spi::checksum(uint8_t* paquete, size_t tamaño){
	LL_CRC_ResetCRCCalculationUnit(CRC);
	for (size_t i = 0; i < tamaño - 1; i++){
		LL_CRC_FeedData8(CRC, paquete[i]);
	}
	return LL_CRC_ReadData8(CRC);
}

//recibir reporte
void Spi::armarReporte(Stm_t *reporte, uint8_t tcrt1, uint8_t tcrt2, uint8_t tcrt3, uint8_t tcrt4){
	reporte->inicio[0] = HEADER_1;
	reporte->inicio[1] = HEADER_2;
	reporte->inicio[2] = HEADER_3;
	reporte->id = ID_STM;
	reporte->tcrt_1 = tcrt1;
	reporte->tcrt_2 = tcrt2;
	reporte->tcrt_3 = tcrt3;
	reporte->tcrt_4 = tcrt4;
	reporte->cont = cont;
	reporte->final = checksum((uint8_t*)reporte, sizeof(Stm_t));
	cont++;
}

uint8_t Spi::recibirReporte(uint8_t *reporte){
	if(reporte == nullptr){
		return 0;
	}
	if(reporte[0] != HEADER_1 || reporte[1] != HEADER_2 || reporte[2] != HEADER_3){
		//reporte invalido
		return 0;
	}
	if(reporte[3] == ID_CONF){
		Conf_t* conf = (Conf_t*)reporte;
		if(checksum(reporte, sizeof(Conf_t)) != conf->final){
			return 0;
		}

		//aplicacion del umbral del limite
		LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, conf->u_limite);
		LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_2, conf->u_limite);
		static Ok_t ok{};
		ok.inicio[0] = HEADER_1;
		ok.inicio[1] = HEADER_2;
		ok.inicio[2] = HEADER_3;
		ok.id = ID_OK;
		ok.final = checksum((uint8_t*)&ok, sizeof(Ok_t));
		enviar((uint8_t*)&ok, sizeof(Ok_t));
		return ID_CONF;
	}else if(reporte[3] == ID_ESP){
		Esp_t* accion = (Esp_t*)reporte;
		if(checksum(reporte, sizeof(Esp_t)) != accion->final){
			return 0;
		}
		if(accion->banderas & UNLOCK_M){
			TIM1->BDTR |= TIM_BDTR_MOE;
		}
		velObjetivo_1 = accion->obj_1;
		velObjetivo_2 = accion->obj_2;
		return ID_ESP;
	}
	return 0;
}

//envia mensajea
void Spi::enviar(uint8_t *data, uint16_t size) {
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
	while(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2)){

	}
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)data);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, size);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
	LL_SPI_EnableDMAReq_TX(SPI1);
	LL_SPI_Enable(SPI1);
}

//recibe mensjes
void Spi::recibir() {
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
	while(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_1)){

	}
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)bufferDma);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, (MAX_PACKET_SIZE*2));
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
	LL_SPI_EnableDMAReq_RX(SPI1);
	LL_SPI_Enable(SPI1);
}

Spi spi;

extern "C" void SPI_DMA_RX_Callback(void) {
	uint8_t* paquete = nullptr;
	if (LL_DMA_IsActiveFlag_HT1(DMA1)) {
		LL_DMA_ClearFlag_HT1(DMA1);
		paquete = (uint8_t*)&spi.bufferDma[0];
		spi.recibirReporte(paquete);
	}else if (LL_DMA_IsActiveFlag_TC1(DMA1)) {
		LL_DMA_ClearFlag_TC1(DMA1);
		paquete = (uint8_t*)&spi.bufferDma[MAX_PACKET_SIZE];
		spi.recibirReporte(paquete);
	}
	if (LL_DMA_IsActiveFlag_TE1(DMA1)) {
			LL_DMA_ClearFlag_TE1(DMA1);
	}
}
