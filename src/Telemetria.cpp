#include "Telemetria.h"
#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "ControlMotores.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include "SensorTof.h"
#include "freertos/portmacro.h"
#include "esp_system.h"
#include <cstdio>

//constructor
Telemetria::Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* u, Mqtt* q):
	me(e),
	cm(m),
	sc(c),
	su(u),
	mq(q)
{}

void Telemetria::recopilar(){
	d.estado = me->estado();
	cm->velocidades(&d.pwm1, &d.pwm2);
	sc->colores(&d.scR, &d.scG, &d.scB, &d.scR1, &d.scG1, &d.scB1, &d.scR2, &d.scG2, &d.scB2);
	su->distancias(&d.ojos1, &d.ojos2);
	d.ToF1 = 0;
	d.ToF2 = 0;
	d.ToF3 = 0;
	d.ToF4 = 0;
	d.ToF5 = 0;
	d.ToF6 = 0;
	d.pila = 0;
	d.tiempo = xTaskGetTickCount() * portTICK_PERIOD_MS;
	d.heap = esp_get_free_heap_size();
}

void Telemetria::enviar(){
	//se crea el strcut
	recopilar();

	//variable que guardara el json
	char json[NJSON];

	//se arma el json
	int lon = snprintf(json, NJSON,
    "{\"t\":%lu,\"e\":%d,\"h\":%lu,\"m\":[%d,%d],\"u\":[%d,%d],\"c1\":[%d,%d,%d],\"c2\":[%d,%d,%d],\"ref\":[%d,%d,%d]}",
    d.tiempo, d.estado, d.heap, // Heap con %lu
    d.pwm1, d.pwm2,
    d.ojos1, d.ojos2,
    d.scR1, d.scG1, d.scB1,
    d.scR2, d.scG2, d.scB2,
    d.scR, d.scG, d.scB
	);

	//se valida y envia el json
	if(lon > 0){
		mq->pub(json, "robot/telemetria", 0, 0);
	}
}
