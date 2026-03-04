#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_bit_defs.h"

#define WIFI CONFIG_WIFI_SSID
#define WIFI_PASS_CRED CONFIG_WIFI_PASSWORD
#define WIFI_CONNECTED BIT0

class Wifi{
	private:
		EventGroupHandle_t e;
		int intentos;
		void wifi();
		static void evento(void* arg, esp_event_base_t base, int32_t id, void* data);
		//metodo para el mDNS
		static void mdns();
		//metodo para smartconfig
		void smart();
	public:
		void begin();
		void espera();
		void signalW(int* _signal);
};
#endif
