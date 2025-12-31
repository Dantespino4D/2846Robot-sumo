#include "Mqtt.h"
#include "Ota.h"
#include "esp_event_base.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <cstdint>
#include <string>

static const char *TAG = "MQTT";

void Mqtt::begin(){
	//se configura
	esp_mqtt_client_config_t mqttC = {};
	mqttC.broker.address.uri = BROKER;

	//se inicializa
	this->cliente = esp_mqtt_client_init(&mqttC);

	//se registra la funcion que recibira los eventos
	esp_mqtt_client_register_event(this->cliente, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, Mqtt::evento, this);

	//inicia
	esp_mqtt_client_start(cliente);
}

void Mqtt::evento(void* arg, esp_event_base_t base, int32_t id, void* data){
	//se guarda el obtejo
	Mqtt* self = (Mqtt*)arg;

	//guarda el evento en esta variable
	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)data;

	//decide que hacer con el evento
	switch((esp_mqtt_event_id_t)id){
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "mqtt exitoso");
			esp_mqtt_client_subscribe(self->cliente, "ota", 1);
			self->pub("funciona", "ota/log", 1, 0);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGE(TAG, "mqtt se desconecto");
			break;
		case MQTT_EVENT_DATA:
			if(strncmp(event->topic, "ota", event->topic_len) == 0){
				ESP_LOGI(TAG, "url recibida");
				char *urlT = (char*)malloc(event->data_len + 1);
				if(urlT == NULL){
					ESP_LOGE(TAG, "error al procesar url");
					break;
				}
				memcpy(urlT, event->data, event->data_len);
                urlT[event->data_len] = '\0';

				Ota ota;
				ota.ota(urlT);

				free(urlT);
			}
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "mqtt fallo");
			break;
		default:
			break;
	}


}
void Mqtt::pub(const std::string& men, const std::string& top, int q, int r){
	esp_mqtt_client_publish(cliente, top.c_str(), men.c_str(), 0, q, r);
}

