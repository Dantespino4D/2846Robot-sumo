#include "esp_timer.h"
#include "SensorRival.h"
#include "Nvs.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

SensorRival::SensorRival(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2):
	trig_1(_trig_1),
	trig_2(_trig_2),
	echo_1(_echo_1),
	echo_2(_echo_2),
    maxd(_maxd),
	ind1(0),
	ind2(0),
	total1(0),
	total2(0)
{
for(int i = 0; i < N_MUESTRAS; i++) {
        mem1[i] = 0;
        mem2[i] = 0;
    }
}

uint16_t SensorRival::dist_cm(gpio_num_t trig, gpio_num_t echo){
	//configuracion inicial
	gpio_set_level(trig, 0);
	ets_delay_us(2);

	//pulso trigger
	gpio_set_level(trig, 1);
	ets_delay_us(10);
	gpio_set_level(trig, 0);

	//detectar por echo
	uint32_t timeout_us = 25000;
	int64_t startT = esp_timer_get_time();
	while(gpio_get_level(echo) == 0){
		if((esp_timer_get_time() - startT) > timeout_us){
			return 0;
		}
	}

	//mide el tiempo en el que se detecta por echo
	startT = esp_timer_get_time();
	while(gpio_get_level(echo) == 1){
		if((esp_timer_get_time() - startT) > timeout_us){
			return 0;
		}
	}
	//calcula la distacia
	uint64_t dur = esp_timer_get_time() - startT;
	uint32_t dis = dur / 58;
	//retorna la distancia
	return dis;
}

uint16_t SensorRival::filtro(gpio_num_t trig, gpio_num_t echo, uint16_t* mem, int& ind, long& total){
	//se eliminaba del total la lectura mas vieja
	total = total - mem[ind];

	//se le la nueva lectura
	int lec = dist_cm(trig, echo);

	//se agrega a la lista
	mem[ind] = lec;

	//se agrega al total
	total = total + mem[ind];

	//se suma 1 al indice o lo restablecemos a 0 si llega al maximo
	ind++;
	if(ind >= N_MUESTRAS){
		ind = 0;
	}

	//retorna el promedio
	return total / N_MUESTRAS;
}

//metodo que verifica ojos 1
bool SensorRival::ojos_1Verify(){
	dis2 = filtro(trig_1, echo_1, mem1, ind1, total1);
	return (dis2 > 0 && dis2 <= maxd);
}

//metodo que verifica ojos 2
bool SensorRival::ojos_2Verify(){
	dis1 = filtro(trig_2, echo_2, mem2, ind2, total2);
	return (dis1 > 0 && dis1 <= maxd);
}

//metodo que lee de la nvs la distancia maxima guardada
void SensorRival::nvsLeer(){
	Nvs nvs("sensores");
	maxd = nvs.leer("dist_max", maxd);
}

//metodo que da las distancias para telemetria
void SensorRival::distancias(uint16_t* d1, uint16_t* d2){
			*d1 = dis1;
			*d2 = dis2;
}
