#ifndef SPI_H
#define SPI_H

#include "protocolo/sumo_protocol.h"
#include "driver/spi_master.h"
#include <cstdint>
#include <string.h>
#include "esp_log.h"

class Spi {
	private:
		uint8_t* tx;
		uint8_t* rx;
		Stm_t telemetria;
		Esp_t orden;
		Ok_t ok;
		Conf_t conf;

		spi_transaction_t transmision;
		spi_device_interface_config_t spistm32;
		spi_device_handle_t stm32_handle;
		bool transaccionEnCurso;
		uint8_t cont;
		uint8_t fallos;
		void enviarRecibir(uint8_t* mensaje, uint8_t* respuesta, size_t size);
	public:
		Spi();
		void begin();
		void armarOrden(int16_t obj_1, int16_t obj_2);
		void enviarConfiguracion();
		void procesarRespuesta();
		void recolectar();
		Stm_t getTelemetria() const;
};

#endif // SPI_H
