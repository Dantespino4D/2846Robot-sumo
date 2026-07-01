#include "app_main.h"
#include "Motores.h"
#include "tim.h"

void app_main(void)
{
	//se establece la accioninicial como en alto
	uint8_t accion = 0;
	//se crea el objetp de los motores
	Motores mot(&htim1);
	while (1)
	{
		//se manda la nueva accion
		mot.controlador(accion);
	}
}
