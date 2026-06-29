#ifndef SUMO_PROTOCOL_H
#define SUMO_PROTOCOL_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    uint8_t estado_maquina;
    int16_t setpoint_izq;
    int16_t setpoint_der;
    uint8_t flag_linea;
    uint8_t checksum;
} MasterToSlave_Msg_t;

typedef struct {
    uint8_t estado_maquina;
    int16_t setpoint_izq;
    int16_t setpoint_der;
    uint8_t flag_linea;
    uint8_t checksum;
} SlaveToMaster_Msg_t;

#pragma pack(pop)

#endif // SUMO_PROTOCOL_H
