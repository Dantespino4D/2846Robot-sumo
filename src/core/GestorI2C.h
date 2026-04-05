#ifndef GESTORI2C_H
#define GESTORI2C_H

#include "driver/i2c.h"
#include "sdkconfig.h"

#define ERR_MAX 10

class GestorI2C {
    private:
        i2c_config_t conf;
        int err;
    public:
        GestorI2C();
        void begin();
        void error();
        void reset();
        bool verify();
        void reinicio();
        i2c_port_t port() const;
};

#endif
