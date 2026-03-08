#ifndef ESTRATEGIA_PROTOTIPO_H
#define ESTRATEGIA_PROTOTIPO_H

#include "EstrategiaBase.h"

class MaquinaEstados;

class EstrategiaPrototipo : public EstrategiaBase {
public:
    void seleccion(MaquinaEstados* ctx) override;
    void ejecucion(MaquinaEstados* ctx) override;
};

#endif
