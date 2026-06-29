#ifndef ESTRATEGIA2_H
#define ESTRATEGIA2_H

#include "EstrategiaEstandar.h"

class MaquinaEstados;

class Estrategia2 : public EstrategiaEstandar {
	public:
		void ejecucion(MaquinaEstados* ctx) override;
};

#endif
