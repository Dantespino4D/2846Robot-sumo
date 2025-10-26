#include <Arduino.h>
#include <NewPing.h>
#include "SensorRival.h"

SensorRival::SensorRival(int _maxd, int _trig_1, int _echo_1, int _trig_2, int _echo_2):
	maxd(_maxd),
	ojos_1(_trig_1, _echo_1, _maxd),
	ojos_2(_trig_2, _echo_2, _maxd),
	trig_1(_trig_1),
	trig_2(_trig_2),
	echo_1(_echo_1),
	echo_2(_echo_2)

{}

bool SensorRival::ojos_1Verify(){
	unsigned int dis = ojos_1.ping_cm();
	return (dis > 0 && dis <= maxd);
}

bool SensorRival::ojos_2Verify(){
	unsigned int dis = ojos_2.ping_cm();
	return (dis > 0 && dis <= maxd);
}
