#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "ControlMotores.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include "MonitorSistema.h"
#include "Wifi.h"

#define NJSON 2048

class Telemetria{
	private:
		Datos d;
		bool final;
		MaquinaEstados* me;
		ControlMotores* cm;
		SensorLimite* sc;
		SensorRival* sr;
		Mqtt* mq;
		Wifi* wf;
		MonitorSistema* ms;
		void sensorLimite();
		void sensorRival();
		void sistema();
		void recopilar();
	public:
		Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* r, Mqtt* q, Wifi* w, MonitorSistema* s, bool _final);
		void enviar();
};

#endif
