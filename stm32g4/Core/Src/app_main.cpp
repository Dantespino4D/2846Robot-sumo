#include "app_main.h"
#include "Motores.h"
#include "Spi.h"
#include "protocolo/sumo_protocol.h"
#include "tim.h"

Spi spi;
extern "C" void gatillo(uint16_t* rampa);

void app_main(void)
{
	//se establece la accioninicial como en alto
	uint8_t accion = 0;
	//se crea el objetp de los motores

	spi.begin();
	while (1)
	{

	}
}
