#ifndef SPI_H
#define SPI_H
#include "stm32g4xx_hal.h"
#include "protocolo/sumo_protocol.h"
#include <stdint.h>

class Spi {
	private:
		uint8_t bufferA[2*MAX_PACKET_SIZE];
		uint8_t bufferB[2*MAX_PACKET_SIZE];
		uint16_t cont;
	public:
		Spi();
		volatile uint8_t *bufferCpu;
		volatile uint8_t *bufferDma;
		volatile bool paquete_N;
		void begin();
		bool nuevoPaquete();
		uint8_t* obtenerlPaquete();
		void marcarProcesado();
		void armarReporte(Stm_t *reporte, uint8_t tcrt1, uint8_t tcrt2, uint8_t tcrt3, uint8_t tcrt4);
		void enviarOk(Ok_t *reporte, Conf_t *configuracion);
		uint8_t recibirReporte(uint8_t *reporte);
		uint8_t checksum(uint8_t* paquete, size_t tamaño);
		void enviar(uint8_t *data, uint16_t size);
		void recibir();
};

#endif
