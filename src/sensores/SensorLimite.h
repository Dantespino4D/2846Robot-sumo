#ifndef SENSORLIMITE_H
#define SENSORLIMITE_H

#include <cstdint>

class SensorLimite {
public:
    virtual ~SensorLimite() {}
    virtual void begin() = 0;
};

#endif
