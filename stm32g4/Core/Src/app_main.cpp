#include "app_main.h"
#include "Motores.h"
#include "tim.h"

void app_main(void)
{
	uint8_t accion = 0;
	Motores mot(&htim1);
	while (1)
	{
		mot.controlador(accion);
	}
}
