#ifndef MULTIPLEXOR_H
#define MULTIPLEXOR_H

#include "GestorI2C.h"

class Multiplexor {
    public:
        Multiplexor();
        bool sel(GestorI2C* i2c, uint8_t canal);
};

#endif
