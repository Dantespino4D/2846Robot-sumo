#ifndef EXPANSORIO_H
#define EXPANSORIO_H
#include "../core/GestorI2C.h"
#include <sys/types.h>

class ExpansorIO {
	private:
		uint8_t direccion;
		uint8_t estado;
		GestorI2C& i2c; // Cambiado a referencia
  	public:
		ExpansorIO(GestorI2C& _i2c); // Cambiado a referencia
		void escribir(uint8_t pin, uint8_t value);
};

#endif
