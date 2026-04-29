#ifndef OTA_H
#define OTA_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define STACK_OTA 12288

class Ota{
	private:
		static void tareaOta(void *pvParameter);
		static StaticTask_t tcbOta;
		static StackType_t stackOta[STACK_OTA];
	public:
		void ota(const char* url);
};

#endif
