#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "ControlMotores.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include "Wifi.h"

#define NJSON 2048

class Telemetria{
	private:
		//struct con los datos
		Datos d;
		//punteros de los objetos
		MaquinaEstados* me;
		ControlMotores* cm;
		SensorLimite* sc;
		SensorRival* sr; // Polimorfismo!
		Mqtt* mq;
		Wifi* wf;
		//se arma el struct
		void recopilar();
	public:
		//constructor
		Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* r, Mqtt* q, Wifi* w);
		//envia el stuct
		void enviar();
};

#endif