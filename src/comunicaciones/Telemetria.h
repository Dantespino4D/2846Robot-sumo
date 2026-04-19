#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include "../core/DatosT.h"
#include "Mqtt.h"
#include "../core/MaquinaEstados.h"
#include "../actuadores/ControlMotores.h"
#include "../sensores/SensorLimite.h"
#include "../sensores/SensorRival.h"
#include "Wifi.h"

#define NJSON 2048

class Telemetria{
	private:
		//struct con los datos
		Datos d;
		//variable de deteccion de verion del hardware
		bool final;
		//punteros de los objetos
		MaquinaEstados* me;
		ControlMotores* cm;
		SensorLimite* sc;
		SensorRival* sr; // Polimorfismo!
		Mqtt* mq;
		Wifi* wf;
		//sensor de limite
		void sensorLimite();
		//sensor rival
		void sensorRival();
		//se arma el struct
		void recopilar();
	public:
		//constructor
		Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* r, Mqtt* q, Wifi* w, bool _final);
		//envia el stuct
		void enviar();
};

#endif
