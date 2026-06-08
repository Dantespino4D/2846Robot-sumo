#include "Telemetria.h"
#include "DatosT.h"
#include "Mqtt.h"
#include "MaquinaEstados.h"
#include "ControlMotores.h"
#include "SensorLimite.h"
#include "Wifi.h"
#include "esp_system.h"
#include <cstdio>
#include <inttypes.h>

#ifndef COMMIT
#define COMMIT 0
#endif

//constructor
Telemetria::Telemetria(MaquinaEstados* e, ControlMotores* m, SensorLimite* c, SensorRival* r, Mqtt* q, Wifi* w, MonitorSistema* s, bool _final):
	d{},
	final(_final),
	me(e),
	cm(m),
	sc(c),
	sr(r),
	mq(q),
	wf(w),
	ms(s)
{}

void Telemetria::sistema(){
	d.pila = ms->bateria();
}

//metodo que accede a los datos de los sensores del limite
void Telemetria::sensorLimite(){
	//se arma el struct con los valores correspondientes
	uint16_t colBuffer[16];
	// Polimorfismo para sensores de limite
	sc->colores(colBuffer);
	if(final){
		// datos para TCRT
		d.Tcrt1 = colBuffer[0];
		d.Tcrt2 = colBuffer[1];
		d.Tcrt3 = colBuffer[2];
		d.Tcrt4 = colBuffer[3];
	}else{
		// datos para sensores de color
		d.cR1 = colBuffer[0];
		d.cG1 = colBuffer[1];
		d.cB1 = colBuffer[2];
		d.cC1 = colBuffer[3];

		d.cR2 = colBuffer[4];
		d.cG2 = colBuffer[5];
		d.cB2 = colBuffer[6];
		d.cC2 = colBuffer[7];

		d.scR1 = colBuffer[8];
		d.scG1 = colBuffer[9];
		d.scB1 = colBuffer[10];
		d.scC1 = colBuffer[11];

		d.scR2 = colBuffer[12];
		d.scG2 = colBuffer[13];
		d.scB2 = colBuffer[14];
		d.scC2 = colBuffer[15];
	}
}

//metodo que accede a los datos de los sensores del rival
void Telemetria::sensorRival(){
	//se arma el struct con los valores correspondientes
	uint16_t distBuffer[24];
	sr->getDistancias(distBuffer);
	if(final){
		//distancias para sensores ToF
		d.ToF1 = distBuffer[0];
		d.ToF2 = distBuffer[1];
		d.ToF3 = distBuffer[2];
		d.ToF4 = distBuffer[3];
		d.ToF5 = distBuffer[4];
		d.ToF6 = distBuffer[5];

	    // Fiabilidad
		d.estadoToF1 = distBuffer[6];
		d.estadoToF2 = distBuffer[7];
		d.estadoToF3 = distBuffer[8];
		d.estadoToF4 = distBuffer[9];
		d.estadoToF5 = distBuffer[10];
		d.estadoToF6 = distBuffer[11];

		d.señalTof1 = distBuffer[12];
		d.señalTof2 = distBuffer[13];
		d.señalTof3 = distBuffer[14];
		d.señalTof4 = distBuffer[15];
		d.señalTof5 = distBuffer[16];
		d.señalTof6 = distBuffer[17];

		d.ambienteToF1 = distBuffer[18];
		d.ambienteToF2 = distBuffer[19];
		d.ambienteToF3 = distBuffer[20];
		d.ambienteToF4 = distBuffer[21];
		d.ambienteToF5 = distBuffer[22];
		d.ambienteToF6 = distBuffer[23];

	}else{
		//distancias para sensores ultrasónicos
		d.ojos1 = distBuffer[0];
		d.ojos2 = distBuffer[1];
	}
}

void Telemetria::recopilar(){
	//se arma el struct con los valores correspondientes
	bool _stall;
	me->datos(&d.estado, &d.estrategia, &d.ciclo, &d.inicio, &_stall);
	d.stall = _stall ? 1 : 0;
	d.corriente = me->corrienteA;
	wf->signalW(&d.wifi);
	cm->velocidades(&d.pwm1, &d.pwm2, &d.pwm1_obj, &d.pwm2_obj);

	//datos de sensores del rival
	sensorRival();

	//datos de sensores del limite
	sensorLimite();

	//dato del sitema
	sistema();

	//datos generales
	d.prototipo = !final;
	d.tiempo = xTaskGetTickCount() * portTICK_PERIOD_MS;
	d.heap = esp_get_free_heap_size();
}

void Telemetria::enviar(){
	//se crea el strcut
	recopilar();

	//variable que guardara el json
	char json[NJSON];
	int lon = 0;
	//se arma el json
	if(final){
		//json del robot final con métricas de fiabilidad extendidas
		lon = snprintf(json, NJSON,
    	"{\"sistema\":{\"commit\":%d,\"tiempo\":%" PRIu32 ",\"heap\":%" PRIu32 ",\"pila\":%f,\"wifi\":%d,\"ciclo\":%d,\"prototipo\":%d},\"estado\":{\"modo\":%d,\"estrategia\":%d,\"inicio\":%d},\"motores\":{\"pwm_izq\":%d,\"pwm_der\":%d, \"pwm_izq_obj\":%d, \"pwm_der_obj\":%d,\"stall\":%d,\"corriente\":%f},\"sensores\":{\"tof\":[%d,%d,%d,%d,%d,%d],\"f_estado\":[%d,%d,%d,%d,%d,%d],\"f_señal\":[%d,%d,%d,%d,%d,%d],\"f_amb\":[%d,%d,%d,%d,%d,%d],\"tcrt\":[%d,%d,%d,%d]}}",
    	COMMIT, d.tiempo, d.heap, d.pila, d.wifi, d.ciclo ,d.prototipo,
		d.estado, d.estrategia, d.inicio,
    	d.pwm1, d.pwm2, d.pwm1_obj, d.pwm2_obj, d.stall, d.corriente,
    	d.ToF1, d.ToF2, d.ToF3, d.ToF4, d.ToF5, d.ToF6,
		d.estadoToF1, d.estadoToF2, d.estadoToF3, d.estadoToF4, d.estadoToF5, d.estadoToF6,
		d.señalTof1, d.señalTof2, d.señalTof3, d.señalTof4, d.señalTof5, d.señalTof6,
		d.ambienteToF1, d.ambienteToF2, d.ambienteToF3, d.ambienteToF4, d.ambienteToF5, d.ambienteToF6,
		d.Tcrt1, d.Tcrt2, d.Tcrt3, d.Tcrt4
    	);
	}else{
		//json del prototipo
		lon = snprintf(json, NJSON,
		"{\"sistema\":{\"commit\":%d,\"tiempo\":%" PRIu32 ",\"heap\":%" PRIu32 ",\"pila\":%f,\"wifi\":%d,\"ciclo\":%d,\"prototipo\":%d},\"estado\":{\"modo\":%d,\"estrategia\":%d,\"inicio\":%d},\"motores\":{\"pwm_izq\":%d,\"pwm_der\":%d, \"pwm_izq_obj\":%d, \"pwm_der_obj\":%d, \"corriente\":%f},\"sensores\":{\"ultra_del\":%d,\"ultra_atr\":%d,\"referencia_del\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"referencia_atr\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"col_del\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d},\"col_atr\":{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d}}}",
		COMMIT, d.tiempo, d.heap, d.pila, d.wifi, d.ciclo ,d.prototipo,
		d.estado, d.estrategia, d.inicio,
		d.pwm1, d.pwm2, d.pwm1_obj, d.pwm2_obj, d.corriente,
		d.ojos1, d.ojos2,
		d.cR1, d.cG1, d.cB1, d.cC1,
		d.cR2, d.cG2, d.cB2, d.cC2,
		d.scR1, d.scG1, d.scB1, d.scC1,
		d.scR2, d.scG2, d.scB2, d.scC2
		);
	}
	//se valida y envia el json
	if(lon > 0){
		mq->pub(json, "robot/telemetria", 0, 0);
	}
}
