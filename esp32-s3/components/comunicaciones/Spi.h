#ifndef SPI_H
#define SPI_H

#include "driver/spi_master.h"
#include <cstdint>
#include <string.h>
#include "esp_log.h"

class Spi {
	private:
		spi_transaction_t transmision;
		spi_device_interface_config_t spistm32;
		spi_device_handle_t stm32_handle;
		bool transaccionEnCurso;
		uint8_t cont;
		uint8_t fallos;
		void reiniciar();
		void enviarRecibir(uint8_t* mensaje, uint8_t* respuesta, size_t size);
	public:
		Spi();
		void begin();
		void armarOrden(int16_t obj_1, int16_t obj_2, uint8_t ban);
		void enviaConfiguracion();
};

#endif // SPI_H
