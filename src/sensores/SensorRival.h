#ifndef SENSORRIVAL_H
#define SENSORRIVAL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class SensorRival {
	public:
		virtual ~SensorRival() {}
		virtual bool begin() = 0;
		virtual void getDistancias(uint16_t* buffer) = 0;
};
#endif // SENSORRIVAL_H
