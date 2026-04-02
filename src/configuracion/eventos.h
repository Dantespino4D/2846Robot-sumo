#ifndef EVENTOS_H
#define EVENTOS_H
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

//event groups global
extern EventGroupHandle_t eventos;

//bits de sensores de color
#define BIT_SC_1 (1 << 0)
#define BIT_SC_2 (1 << 1)

//bits de sensores TOF direccion A
#define BIT_TOF_AI (1 << 2)
#define BIT_TOF_AC (1 << 3)
#define BIT_TOF_AD (1 << 4)

//bits de sensores TOF direccion B
#define BIT_TOF_BI (1 << 5)
#define BIT_TOF_BC (1 << 6)
#define BIT_TOF_BD (1 << 7)

//arreglo con todos los bits de sensores TOF
static const EventBits_t TOF_BITS[6] = {
	BIT_TOF_AI,
	BIT_TOF_AC,
	BIT_TOF_AD,
	BIT_TOF_BI,
	BIT_TOF_BC,
	BIT_TOF_BD
};

//bits de sensores ultrasonicos
#define BIT_ULTRA_A (1 << 8)
#define BIT_ULTRA_B (1 << 9)

//mascara de bits para sensores de color
#define MASK_COLOR (BIT_SC_1 | BIT_SC_2)

//mascara de bits para sensores TOF direccion A
#define MASK_TOF_A (BIT_TOF_AI | BIT_TOF_AC | BIT_TOF_AD)

//mascara de bits para sensores TOF direccion B
#define MASK_TOF_B (BIT_TOF_BI | BIT_TOF_BC | BIT_TOF_BD)

//mascara de bits para sensores ultrasonicos
#define MASK_ULTRA (BIT_ULTRA_A | BIT_ULTRA_B)

//mascaras de convinaciones de TOFs

//mascara de bits TOFs 1 y 2
#define MASK_TOF_1_2 (BIT_TOF_AI | BIT_TOF_AC)

//mascara de bits TOFs 1 y 3
#define MASK_TOF_1_3 (BIT_TOF_AI | BIT_TOF_AD)

//mascara de bits TOFs 2 y 3
#define MASK_TOF_2_3 (BIT_TOF_AC | BIT_TOF_AD)

//mascara de bits TOFs 4 y 5
#define MASK_TOF_4_5 (BIT_TOF_BI | BIT_TOF_BC)

//mascara de bits TOFs 4 y 6
#define MASK_TOF_4_6 (BIT_TOF_BI | BIT_TOF_BD)

//mascara de bits TOFs 5 y 6
#define MASK_TOF_5_6 (BIT_TOF_BC | BIT_TOF_BD)

#endif // EVENTOS_H
