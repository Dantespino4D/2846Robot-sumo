#ifndef ESTRATEGIA_BASE_H
#define ESTRATEGIA_BASE_H

class MaquinaEstados; // Forward declaration

class EstrategiaBase {
public:
    virtual void seleccion(MaquinaEstados* ctx) = 0;
    virtual void ejecucion(MaquinaEstados* ctx) = 0;
    virtual ~EstrategiaBase() {}
};

#endif
