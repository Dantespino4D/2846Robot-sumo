#include "stm32g474xx.h"
#include "Spi.h"
#include "protocolo/sumo_protocol.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_crc.h"
#include <cstddef>
#include <cstdint>

extern "C" void gatillo(uint16_t* rampa);

Spi::Spi() :
	bufferA{},
	bufferB{},
	bufferCpu(bufferB),
	paquete_N(false),
	cont(0),
	bufferDma(bufferA)
{}

void Spi::begin() {
	recibir((uint8_t*)bufferDma, MAX_PACKET_SIZE);
}

void Spi::cambioBuffers() {
	if (bufferDma == bufferA) {
		bufferDma = bufferB;
		bufferCpu = bufferA;
	} else {
		bufferDma = bufferA;
		bufferCpu = bufferB;
	}

	paquete_N = true;

	recibir((uint8_t*)bufferDma, MAX_PACKET_SIZE);
}

bool Spi::nuevoPaquete() {
	return paquete_N;
}

uint8_t* Spi::obtenerlPaquete() {
	return (uint8_t*)bufferCpu;
}

void Spi::marcarProcesado() {
	paquete_N = false;
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
	if(reporte[0] != HEADER_1 || reporte[1] != HEADER_2){
		//reporte invalido
		return 0;
	}
	if(reporte[2] == ID_CONF){
		Conf_t* conf = (Conf_t*)reporte;
		if(checksum(reporte, sizeof(Conf_t)) != conf->final){
			return 0;
		}
		static Ok_t ok{};
		ok.inicio[0] = HEADER_1;
		ok.inicio[1] = HEADER_2;
		ok.id = ID_OK;
		ok.final = checksum((uint8_t*)&ok, sizeof(Ok_t));
		enviar((uint8_t*)&ok, sizeof(Ok_t));
		return ID_CONF;
	}else if(reporte[2] == ID_ESP){
		Esp_t* accion = (Esp_t*)reporte;
		if(checksum(reporte, sizeof(Esp_t)) != accion->final){
			return 0;
		}
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
void Spi::recibir(uint8_t *data, uint16_t size) {
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
	while(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_1)){

	}
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)data);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, size);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
	LL_SPI_EnableDMAReq_RX(SPI1);
	LL_SPI_Enable(SPI1);
}

extern Spi spi;

extern "C" void SPI_DMA_RX_Callback(void) {
	if (LL_DMA_IsActiveFlag_TC1(DMA1)) {
		LL_DMA_ClearFlag_TC1(DMA1);
		uint8_t* paquete = (uint8_t*)spi.bufferDma;
		if (paquete[0] == HEADER_1 && paquete[1] == HEADER_2 && paquete[2] == ID_ESP) {
			Esp_t* accion = (Esp_t*)paquete;
			if (spi.checksum(paquete, sizeof(Esp_t)) == accion->final) {
				gatillo(accion->pwm);
			}
		}
		spi.cambioBuffers();
	}
	if (LL_DMA_IsActiveFlag_TE1(DMA1)) {
		LL_DMA_ClearFlag_TE1(DMA1);
	}
}
