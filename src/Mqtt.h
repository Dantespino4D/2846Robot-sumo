#ifndef MQTT_H
#define MQTT_H

#include "esp_event_base.h"
#include "mqtt_client.h"
#include <cstdint>
#include <string>

#define BROKER "mqtt://192.168.1.69"

class Mqtt{
	private:
		esp_mqtt_client_handle_t cliente;

		static void evento(void* arg, esp_event_base_t base, int32_t id, void* data);

	public:
		void begin();
		void pub(const std::string& men, const std::string& top, int q, int r);
};

#endif
