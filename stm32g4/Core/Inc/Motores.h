#ifndef MOTORES_H
#define MOTORES_H

#include <cstdint>

class Motores {
	public:
		//constructor
		Motores();

		//metodo de inicializacion
		void begin();

		//metodo que controla la velocidad
		void ejecutarAccion(uint16_t* rampa);
};

#endif // MOTORES_H
