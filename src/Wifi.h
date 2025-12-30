#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_bit_defs.h"

#define WIFI "INFINITUM288C_2.4"
#define CONTRASEÑA "2465364457"
#define WIFI_CONNECTED BIT0

class Wifi{
	private:
		EventGroupHandle_t e;
		void wifi();
		static void evento(void* arg, esp_event_base_t base, int32_t id, void* data);
	public:
		void begin();
		void espera();
};
#endif
