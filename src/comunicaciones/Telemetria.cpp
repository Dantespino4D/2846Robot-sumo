#include "Telemetria.h"
#include "../core/DatosT.h"
#include "Mqtt.h"
#include "../core/MaquinaEstados.h"
#include "../actuadores/ControlMotores.h"
#include "../sensores/SensorLimite.h"
#include "Wifi.h"
#include "esp_system.h"
#include <cstdio>
#include <inttypes.h>

#ifndef COMMIT
#define COMMIT 0
#endif

//constructor
Telemetria::Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* r, Mqtt* q, Wifi* w):
	d{},
	me(e),
	cm(m),
	sc(c),
	sr(r),
	mq(q),
	wf(w)
{}

void Telemetria::recopilar(){
	//se arma el struct con los valores correspondientes
	me->datos(&d.estado, &d.estrategia, &d.ciclo, &d.inicio);
	wf->signalW(&d.wifi);
	cm->velocidades(&d.pwm1, &d.pwm2);

	// Polimorfismo para sensores de limite
	uint16_t colBuffer[16];
	sc->colores(colBuffer);

	d.cR1 = colBuffer[0]; d.cG1 = colBuffer[1]; d.cB1 = colBuffer[2]; d.cC1 = colBuffer[3];
	d.cR2 = colBuffer[4]; d.cG2 = colBuffer[5]; d.cB2 = colBuffer[6]; d.cC2 = colBuffer[7];
	d.scR1 = colBuffer[8]; d.scG1 = colBuffer[9]; d.scB1 = colBuffer[10]; d.scC1 = colBuffer[11];
	d.scR2 = colBuffer[12]; d.scG2 = colBuffer[13]; d.scB2 = colBuffer[14]; d.scC2 = colBuffer[15];

	// Caso específico para TCRT (primeros 2 valores del buffer)
	d.Tcrt1 = colBuffer[0];
	d.Tcrt2 = colBuffer[1];

    // Polimorfismo: Obtenemos todas las lecturas en un buffer unificado
    uint16_t distBuffer[8];
	sr->getDistancias(distBuffer);

    // Mapeamos según tu esquema: [0-1]=Ultra, [2-7]=ToF
    d.ojos1 = distBuffer[0];
    d.ojos2 = distBuffer[1];
	d.ToF1 = distBuffer[2];
	d.ToF2 = distBuffer[3];
	d.ToF3 = distBuffer[4];
	d.ToF4 = distBuffer[5];
	d.ToF5 = distBuffer[6];
	d.ToF6 = distBuffer[7];

    // Fiabilidad y otros datos
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

	//se arma el json (agregando TCRTs)
	int lon = snprintf(json, NJSON,
    "{\"sistema\":{\"commit\":%d,\"tiempo\":%"PRIu32",\"heap\":%"PRIu32",\"pila\":%f,\"temp\":%f,\"wifi\":%d,\"ciclo\":%d,\"prototipo\":%d},\"estado\":{\"modo\":%d,\"estrategia\":%d,\"inicio\":%d},\"motores\":{\"pwm_izq\":%d,\"pwm_der\":%d,\"stall\":%d},\"sensores\":{\"ultra_del\":%d,\"ultra_atr\":%d,\"tof_del\":[%d,%d,%d],\"fiabilidad_del\":[%d,%d,%d],\"tof_atr\":[%d,%d,%d],\"fiabilidad_atr\":[%d,%d,%d],\"referencia_del\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"referencia_atr\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"col_del\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"col_atr\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"tcrt\":[%d,%d]}}",
    COMMIT, d.tiempo, d.heap, d.pila, d.temperatura, d.wifi, d.ciclo ,d.prototipo,
	d.estado, d.estrategia, d.inicio,
    d.pwm1, d.pwm2, d.stall,
    d.ojos1, d.ojos2, d.ToF1, d.ToF2, d.ToF3, d.fToF1, d.fToF2, d.fToF3, d.ToF4, d.ToF5, d.ToF6, d.fToF4, d.fToF5, d.fToF6,
	d.cR1, d.cG1, d.cB1, d.cC1,
	d.cR2, d.cG2, d.cB2, d.cC2,
    d.scR1, d.scG1, d.scB1, d.scC1,
    d.scR2, d.scG2, d.scB2, d.scC2,
	d.Tcrt1, d.Tcrt2
    	);

	//se valida y envia el json
	if(lon > 0){
		mq->pub(json, "robot/telemetria", 0, 0);
	}
}
