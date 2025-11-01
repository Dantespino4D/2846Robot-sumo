#include "esp_timer.h"
#include "SensorRival.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

SensorRival::SensorRival(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2):
	trig_1(_trig_1),  // <-- Mover antes de maxd
    trig_2(_trig_2),  // <-- Mover antes de maxd
    echo_1(_echo_1),  // <-- Mover antes de maxd
    echo_2(_echo_2),  // <-- Mover antes de maxd
    maxd(_maxd)       // <-- 'maxd' debe ir al final

{}

uint32_t SensorRival::dist_cm(gpio_num_t trig, gpio_num_t echo){
	//configuracion inicial
	gpio_set_level(trig, 0);
	ets_delay_us(2);

	//pulso trigger
	gpio_set_level(trig, 1);
	ets_delay_us(10);
	gpio_set_level(trig, 0);

	//detectar por echo
	uint32_t timeout_us = 50000;
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

bool SensorRival::ojos_1Verify(){
	unsigned int dis = dist_cm(trig_1, echo_1);
	return (dis > 0 && dis <= maxd);
}

bool SensorRival::ojos_2Verify(){
	unsigned int dis = dist_cm(trig_2, echo_2);
	return (dis > 0 && dis <= maxd);
}
