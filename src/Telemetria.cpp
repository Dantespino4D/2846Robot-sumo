#include "Telemetria.h"
#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "ControlMotores.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include "SensorTof.h"
#include "Wifi.h"
#include "freertos/portmacro.h"
#include "esp_system.h"
#include <cstdio>

#ifndef COMMIT
#define COMMIT 0
#endif

//constructor
Telemetria::Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* u, Mqtt* q, Wifi* w):
	me(e),
	cm(m),
	sc(c),
	su(u),
	mq(q),
	wf(w)
{}

void Telemetria::recopilar(){
	//se arma el struct con los valores correspondientes
	me->datos(&d.estado, &d.estrategia, &d.ciclo, &d.inicio);
	wf->señalW(&d.wifi);
	cm->velocidades(&d.pwm1, &d.pwm2);
	sc->colores(&d.cR1, &d.cG1, &d.cB1, &d.cC1, &d.cR2, &d.cG2, &d.cB2, &d.cC2, &d.scR1, &d.scG1, &d.scB1, &d.scC1, &d.scR2, &d.scG2, &d.scB2, &d.scC2);
	su->distancias(&d.ojos1, &d.ojos2);
	d.ToF1 = 0;
	d.ToF2 = 0;
	d.ToF3 = 0;
	d.ToF4 = 0;
	d.ToF5 = 0;
	d.ToF6 = 0;
	d.fToF1 = 0;
	d.fToF2 = 0;
	d.fToF3 = 0;
	d.fToF4 = 0;
	d.fToF5 = 0;
	d.fToF6 = 0;

	d.pila = 0;
	d.prototipo = 1;
	d.tiempo = xTaskGetTickCount() * portTICK_PERIOD_MS;
	d.heap = esp_get_free_heap_size();
	d.temperatura = 0;
	d.stall = 0;
}

void Telemetria::enviar(){
	//se crea el strcut
	recopilar();

	//variable que guardara el json
	char json[NJSON];

	//se arma el json
	int lon = snprintf(json, NJSON,
    "{\"sistema\":{\"commit\":%d,\"tiempo\":%lu,\"heap\":%lu,\"pila\":%f,\"temp\":%f,\"wifi\":%d,\"ciclo\":%d,\"prototipo\":%d},\"estado\":{\"modo\":%d,\"estrategia\":%d,\"inicio\":%d},\"motores\":{\"pwm_izq\":%d,\"pwm_der\":%d,\"stall\":%d},\"sensores\":{\"ultra_del\":%d,\"ultra_atr\":%d,\"tof_del\":[%d,%d,%d],\"fiabilidad_del\":[%d,%d,%d],\"tof_atr\":[%d,%d,%d],\"fiabilidad_atr\":[%d,%d,%d],\"referencia_del\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"referencia_atr\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"col_del\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"col_atr\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d}}}",
    COMMIT, d.tiempo, d.heap, d.pila, d.temperatura, d.wifi, d.ciclo ,d.prototipo,
	d.estado, d.estrategia, d.inicio,
    d.pwm1, d.pwm2, d.stall,
    d.ojos1, d.ojos2, d.ToF1, d.ToF2, d.ToF3, d.fToF1, d.fToF2, d.fToF3, d.ToF4, d.ToF5, d.ToF6, d.fToF4, d.fToF5, d.fToF6,
	d.cR1, d.cG1, d.cB1, d.cC1,
	d.cR2, d.cG2, d.cB2, d.cC2,
    d.scR1, d.scG1, d.scB1, d.scC1,
    d.scR2, d.scG2, d.scB2, d.scC2
    	);

	//se valida y envia el json
	if(lon > 0){
		mq->pub(json, "robot/telemetria", 0, 0);
	}
}
