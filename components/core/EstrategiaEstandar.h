#ifndef ESTRATEGIA_ESTANDAR_H
#define ESTRATEGIA_ESTANDAR_H

#include "EstrategiaBase.h"

class EstrategiaEstandar : public EstrategiaBase {
public:
    //selecciona el estado
    void seleccion(MaquinaEstados* ctx) override;

    // El método ejecucion sigue siendo obligatorio para los hijos (E1, E2, etc.)
    virtual void ejecucion(MaquinaEstados* ctx) override = 0;
};

#endif
