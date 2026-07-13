#ifndef SUMO_PROTOCOL_H
#define SUMO_PROTOCOL_H

#include <cstdint>
#include <stdint.h>

#define HEADER_1 0xAA
#define HEADER_2 0x55
#define HEADER_3 0xA5
#define ID_CONF 0x01
#define ID_ESP 0x02
#define ID_STM 0x03
#define ID_OK 0x04
#define UNLOCK_M (1 << 0)
#define MAX_PACKET_SIZE sizeof(Stm_t)


#pragma pack(push, 1)

typedef struct {
	//verificador del inicio de la configuracion
	uint8_t inicio[3];

	//identificador del mensaje
	uint8_t id;

	//umbral de los TCRT
	uint16_t u_limite;

	//relleno
	uint8_t relleno[11];

	//verificador del final de la configuracion
	uint8_t final;
} Conf_t;

typedef struct {
	//verificador del inicio de la accion
	uint8_t inicio[3];

	//identificador del mensaje
	uint8_t id;

	//velocidades objetio
	int16_t obj_1;
	int16_t obj_2;

	//banderas
	uint8_t banderas;

	//contador
	uint8_t cont;

	//relleno
	uint8_t relleno[7];

	//verificador del final de la accion
	uint8_t final;
} Esp_t;

typedef struct {
	//verificador del inicio de la transmision
	uint8_t inicio[3];

	//identificador del mensaje
	uint8_t id;

	//valores de los TCRT5000
    uint8_t tcrt_1;
	uint8_t tcrt_2;
	uint8_t tcrt_3;
	uint8_t tcrt_4;

	//corriente de cada DRV8874
	uint8_t drv_1;
	uint8_t drv_2;
	uint8_t drv_3;
	uint8_t drv_4;

	//bateria
	float bateria;

	//contador
	uint8_t cont;

	//verificador del final de la transmision
	uint8_t final;
} Stm_t;

typedef struct {
	//verificador del inicio de la transmision
	uint8_t inicio[3];

	//identificador del mensaje
	uint8_t id;

	//relleno
	uint8_t relleno[12];

	//verificador del final de la transmision
	uint8_t final;
} Ok_t;
#pragma pack(pop)

#endif // SUMO_PROTOCOL_H
