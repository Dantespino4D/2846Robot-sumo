#ifndef MQTT_H
#define MQTT_H

#include "esp_event_base.h"
#include "mqtt_client.h"
#include <cstdint>
#include "cJSON.h"

#define BROKER CONFIG_MQTT_BROKER
#define USUARIO CONFIG_MQTT_USER
#define PASSWORD CONFIG_MQTT_PASSWORD

class Mqtt{
	private:
		esp_mqtt_client_handle_t cliente;

		static void evento(void* arg, esp_event_base_t base, int32_t id, void* data);
		void configuracion_json(esp_mqtt_event_handle_t evento);
		void extraer(cJSON *padre, const char* clave, const char* name);
		void extraerM(cJSON *padre, const char* item, const char* clave1, const char* clave2);

	public:
		void begin();
		void pub(const char* men, const char* top, int q, int r);
	};

#endif
