#include "app_main.h"
#include "Motores.h"
#include "Spi.h"
#include "protocolo/sumo_protocol.h"
#include "tim.h"

Spi spi;

void app_main(void)
{
	//se establece la accioninicial como en alto
	uint8_t accion = 0;
	//se crea el objetp de los motores
	Motores mot(&htim1);

	spi.begin();
	while (1)
	{
		if(spi.nuevoPaquete()){
			uint8_t* paquete = spi.obtenerlPaquete();
			uint8_t tipo = spi.recibirReporte(spi.obtenerlPaquete());
			if(tipo == ID_ESP){
			Esp_t* accionRecibida = (Esp_t*)paquete;
			accion = accionRecibida->accion;
			} else if(tipo == ID_CONF){

			}
			spi.marcarProcesado();
		}
		//se manda la nueva accion
		mot.controlador(accion);
	}
}
