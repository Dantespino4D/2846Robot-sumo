#ifndef SPI_H
#define SPI_H
#include "stm32g4xx_hal.h"
#include "protocolo/sumo_protocol.h"
#include <cstdint>

class Spi {
	private:
		SPI_HandleTypeDef hspi;
		uint16_t cont = 0;
		uint8_t checksum(uint8_t* paquete, size_t tamaño);
	public:
		void armarReporte(Stm_t *reporte, uint8_t tcrt1, uint8_t tcrt2, uint8_t tcrt3, uint8_t tcrt4);
		void enviarOk(Ok_t *reporte, Conf_t *configuracion);
		uint8_t recibirReporte(uint8_t *reporte);
		void enviar(uint8_t *data, uint16_t size);
		void recibir(uint8_t *data, uint16_t size);
};

#endif
