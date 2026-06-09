#ifndef GESTORI2C_H
#define GESTORI2C_H

#include "driver/i2c_master.h"
#include "sdkconfig.h"
#include <map>

#define ERR_MAX 10

class GestorI2C {
    private:
        i2c_master_bus_handle_t bus_handle;
        std::map<uint8_t, i2c_master_dev_handle_t> devices;
        int err;
    public:
        GestorI2C();
        void begin();
        void error();
        void reset();
        bool verify();
        void reinicio();
        i2c_master_dev_handle_t get_device(uint8_t addr);
        i2c_master_bus_handle_t get_bus() const;
};

#endif
