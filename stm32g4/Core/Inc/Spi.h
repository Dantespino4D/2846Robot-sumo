#ifndef SPI_H
#define SPI_H
#include "stm32g4xx_hal.h"
#include "protocolo/sumo_protocol.h"
#include <stdint.h>

#define MAX_PACKET_SIZE sizeof(Conf_t)

class Spi {
	private:
		uint16_t bufferA[MAX_PACKET_SIZE];
		uint16_t bufferB[MAX_PACKET_SIZE];
		volatile uint16_t *bufferCpu;
		volatile bool paquete_N;
		uint16_t cont;
	public:
		Spi();
		volatile uint16_t *bufferDma;
		void begin();
		bool nuevoPaquete();
		uint8_t* obtenerlPaquete();
		void marcarProcesado();
		void cambioBuffers();
		void armarReporte(Stm_t *reporte, uint8_t tcrt1, uint8_t tcrt2, uint8_t tcrt3, uint8_t tcrt4);
		void enviarOk(Ok_t *reporte, Conf_t *configuracion);
		uint8_t recibirReporte(uint8_t *reporte);
		uint8_t checksum(uint8_t* paquete, size_t tamaño);
		void enviar(uint8_t *data, uint16_t size);
		void recibir(uint8_t *data, uint16_t size);
};

#endif
