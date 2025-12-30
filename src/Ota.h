#ifndef OTA_H
#define OTA_H

#include "esp_log.h"

class Ota{
	private:
		static void tareaOta(void *pvParameter);
	public:
		void ota(const char* url);
};

#endif
