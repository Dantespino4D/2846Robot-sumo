#ifndef SPI_H
#define SPI_H

#include "driver/spi_master.h"
#include <string.h>
#include "esp_log.h"

class Spi {
	private:
		spi_device_handle_t stm32_handle;
	public:
		Spi();
		void begin();
		void enviar();
		void recibir();
};

#endif // SPI_H
