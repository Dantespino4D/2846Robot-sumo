#ifndef MULTIPLEXOR_H
#define MULTIPLEXOR_H

#include "driver/i2c.h"

class Multiplexor {
    public:
        Multiplexor();
        bool sel(i2c_port_t port, uint8_t canal);
};

#endif
