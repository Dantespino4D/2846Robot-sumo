#include "Mqtt.h"
#include "Ota.h"
#include "Nvs.h"
#include "esp_event_base.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"
#include <cstdint>
#include <string>

static const char *TAG = "MQTT";

void Mqtt::begin(){
	//se configura
	esp_mqtt_client_config_t mqttC = {};
	mqttC.broker.address.uri = BROKER;
	mqttC.credentials.username = USUARIO;
	mqttC.credentials.authentication.password = CONTRASEÑA;

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
			esp_mqtt_client_subscribe(self->cliente, "robot/ota", 1);
			esp_mqtt_client_subscribe(self->cliente, "robot/conf", 1);
			self->pub("funciona", "robot/ota/log", 1, 0);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGE(TAG, "mqtt se desconecto");
			break;
		case MQTT_EVENT_DATA:
			if(strncmp(event->topic, "robot/ota", event->topic_len) == 0){
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
			}else if(strncmp(event->topic, "robot/conf", event->topic_len) == 0){
				//se recibe y establece la configuracion enviada por un json
				self->configuracion_json(event);
			}
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "mqtt fallo");
			break;
		default:
			break;
	}


}
void Mqtt::pub(const char* men, const char* top, int q, int r){
	esp_mqtt_client_publish(cliente, top, men, 0, q, r);
}

//metodo para extraer el resto de configuraciones del json para guardarlas en la Nvs
void Mqtt::extraer(cJSON *padre, const char* clave, const char* name){
	//se obtiene el objeto hijo del padre
	cJSON *hijo = cJSON_GetObjectItemCaseSensitive(padre, clave);
	//se verifica si es un numero
	if(cJSON_IsNumber(hijo)){
		//se crea el objeto nvs
		Nvs nvs(name);
		//se lee valores previos
		int val = nvs.leer(clave, -9999);
		//se obtiene el valor entero del json
		int val_j = hijo->valueint;
		if(val != val_j){
			nvs.guardar(clave, val_j);
			ESP_LOGI(TAG, "configuracion nueva recibida:", clave, val_j);
		}
	}
}

//metodo que extrae las velocidades de los motores y las guarda en sus respectivas variables
void Mqtt::extraerM(cJSON *padre, const char* item, const char* clave1, const char* clave2){
	cJSON *hijo = cJSON_GetObjectItemCaseSensitive(padre, item);
	if(hijo != NULL){
		cJSON *izq = cJSON_GetObjectItemCaseSensitive(hijo, "izq");
		cJSON *der = cJSON_GetObjectItemCaseSensitive(hijo, "der");
		Nvs nvs("motores");
		if(cJSON_IsNumber(izq)){
			int vel_izq = nvs.leer(clave1, -9999);

			int izq_v = izq->valueint;
			if (vel_izq != izq_v){
				nvs.guardar(clave1, izq_v);
				ESP_LOGI(TAG, "valocidades de los motores actualizadas");
			}
		}
		if(cJSON_IsNumber(der)){
			int vel_der = nvs.leer(clave2, -9999);

			int der_v = der->valueint;
			if (vel_der != der_v){
				nvs.guardar(clave2, der_v);
			}
		}
	}
}

void Mqtt::configuracion_json(esp_mqtt_event_handle_t evento){
	ESP_LOGI(TAG, "se recibio la configuracion nueva");
	char *jsonT = (char*)malloc(evento->data_len + 1);
	if(jsonT == NULL){
		ESP_LOGE(TAG, "erro al procesar la configuracion del json");
		return;
	}
	memcpy(jsonT, evento->data, evento->data_len);
	jsonT[evento->data_len] = '\0';

	cJSON *maestro = cJSON_Parse(jsonT);
	free(jsonT);
	if(maestro == NULL){
		ESP_LOGE(TAG, "estructura del json invalida");
		return;
	}

	//se revisa y aplican cambios recibidos de la velocidad de los motores
	cJSON *motores = cJSON_GetObjectItemCaseSensitive(maestro, "motores");
	if(motores != NULL){
		extraer(motores, "tiempo_rampa", "motores");
		extraerM(motores, "normal", "velocidad_nI", "velocidad_nD");
		extraerM(motores, "ataque", "velocidad_aI", "velocidad_aD");
		extraerM(motores, "maximo", "velocidad_mI", "velocidad_mD");
		extraerM(motores, "giro", "velocidad_gI", "velocidad_gD");
	}

	//se revisa y aplican cambios en la configuracion de los tiempo
	cJSON *tiempos = cJSON_GetObjectItemCaseSensitive(maestro, "tiempos");
	if(tiempos != NULL){
		extraer(tiempos, "ataque_ciego", "tiempos");
		extraer(tiempos, "retroceso", "tiempos");
		extraer(tiempos, "recta_star", "tiempos");
		extraer(tiempos, "giro_star", "tiempos");
	}

	//se revisa y aplican cambios en la configuracion de los limites de los sensores
	cJSON *sensores = cJSON_GetObjectItemCaseSensitive(maestro, "sensores");
	if(sensores != NULL){
		extraer(sensores, "dist_max", "sensores");
		extraer(sensores, "umbral_color", "sensores");
	}

	//se revisa y aplican cambios en la configuracion del sistema
	cJSON *sistema = cJSON_GetObjectItemCaseSensitive(maestro, "sistema");
	if(sistema != NULL){
		extraer(sistema, "modo", "sistema");
		extraer(sistema, "monitor", "sistema");
		extraer(sistema, "estrategia", "sistema");
	}

	cJSON_Delete(maestro);
	esp_restart();
}
