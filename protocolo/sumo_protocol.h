#ifndef SUMO_PROTOCOL_H
#define SUMO_PROTOCOL_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
	//verificador del inicio de la configuracion
	uint8_t inicio[2];

	//identificador del mensaje
	uint8_t id;

	//umbral de la deteccion del stall
	uint16_t u_stall;

	//tiempo de la rampa
	uint8_t t_ram;

	//velocidadees
	int16_t normal_i;
	int16_t normal_d;
	int16_t ataque_i;
	int16_t ataque_d;
	int16_t pronunciado_i;
	int16_t pronunciado_d;
	int16_t maximo_i;
	int16_t maximo_d;
	int16_t giro_i;
	int16_t giro_d;
	int16_t evasion_i;
	int16_t evasion_d;
	int16_t huida_i;
	int16_t huida_d;

	//umbral de los TCRT
	int16_t u_limite;

	//verificador del final de la configuracion
	uint8_t final;
} Conf_t;

typedef struct {
	//verificador del inicio de la accion
	uint8_t inicio[2];

	//identificador del mensaje
	uint8_t id;

	//accion de los motores
    uint8_t accion;

	//contador
	uint8_t cont;

	//verificador del final de la accion
	uint8_t final;
} Esp_t;

typedef struct {
	//verificador del inicio de la transmision
	uint8_t inicio[2];

	//identificador del mensaje
	uint8_t id;

	//valores de los TCRT5000
    uint8_t tcrt_1;
	uint8_t tcrt_2;
	uint8_t tcrt_3;
	uint8_t tcrt_4;

	//contador
	uint8_t cont;

	//verificador del final de la transmision
	uint8_t final;
} Stm_t;

#pragma pack(pop)

#endif // SUMO_PROTOCOL_H
