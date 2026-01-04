#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "ControlMotores.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include "SensorTof.h"

#define NJSON 512

class Telemetria{
	private:
		//struct con los datos
		Datos d;
		//punteros de los objetos
		MaquinaEstados* me;
		ControlMotores* cm;
		SensorLimite* sc;
		SensorRival* su;
		Mqtt* mq;
		//se arma el struct
		void recopilar();
	public:
		//constructor
		Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* u, Mqtt* q);
		//envia el stuct
		void enviar();
};

#endif
