#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include "../core/DatosT.h"
#include "Mqtt.h"
#include "../core/MaquinaEstados.h"
#include "../actuadores/ControlMotores.h"
#include "../sensores/SensorTcs.h"
#include "../sensores/SensorRival.h"
#include "Wifi.h"

#define NJSON 2048

class Telemetria{
	private:
		//struct con los datos
		Datos d;
		//punteros de los objetos
		MaquinaEstados* me;
		ControlMotores* cm;
		SensorTcs* sc;
		SensorRival* sr; // Polimorfismo!
		Mqtt* mq;
		Wifi* wf;
		//se arma el struct
		void recopilar();
	public:
		//constructor
		Telemetria(MaquinaEstados* e, ControlMotores* m, SensorTcs* c, SensorRival* r, Mqtt* q, Wifi* w);
		//envia el stuct
		void enviar();
};

#endif