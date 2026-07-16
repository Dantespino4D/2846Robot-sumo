#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "Wifi.h"

#define NJSON 2048

class Telemetria{
	private:
		Datos d;
		MaquinaEstados* me;
		Mqtt* mq;
		Wifi* wf;
		
		void recopilar();
	public:
		Telemetria(MaquinaEstados* e, Mqtt* q, Wifi* w);
		void enviar();
};

#endif
