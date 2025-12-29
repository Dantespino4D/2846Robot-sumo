#ifndef MQTT_H
#define MQTT_H

#include "esp_event_base.h"
#include "mqtt_client.h"
#include <cstdint>

#define BROKER "192.168.1.66"

class Mqtt{
	private:
		esp_mqtt_client_handle_t cliente;

		static void evento(void* arg, esp_event_base_t base, int32_t id, void* data);

	public:
		void begin();
};

#endif
