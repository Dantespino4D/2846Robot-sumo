#ifndef ESTRATEGIA1_H
#define ESTRATEGIA1_H

#include "EstrategiaEstandar.h"

class MaquinaEstados;

class Estrategia1 : public EstrategiaEstandar {
public:
	void ejecucion(MaquinaEstados* ctx) override;
};

#endif
