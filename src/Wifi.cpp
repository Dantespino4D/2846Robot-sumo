#include "Wifi.h"
#include "freertos/FreeRTOS.h"
#include "esp_bit_defs.h"
#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "freertos/projdefs.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <cstring>
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "mdns.h"

const char* TAG = "wifi";

void Wifi::begin(){
	//crear un grupo de eventos
	e = xEventGroupCreate();

	esp_netif_create_default_wifi_sta();

	wifi();
}

void Wifi::wifi(){
	//reservar la memoria
	wifi_init_config_t conf = WIFI_INIT_CONFIG_DEFAULT();

	//verififcar si hay algun error
	ESP_ERROR_CHECK(esp_wifi_init(&conf));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::evento, this, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi::evento, this, NULL));

	//se establece en modo sta
	esp_wifi_set_mode(WIFI_MODE_STA);
	wifi_config_t w = {
		.sta = {
			.ssid = 0,
			.password = 0,
			.threshold = {
				.authmode = WIFI_AUTH_WPA2_PSK,
			},
		}
	};

	strncpy((char*)w.sta.ssid, WIFI, 32);
	strncpy((char*)w.sta.password, CONTRASEÑA, 64);

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &w));
	esp_wifi_start();
	//mobjeto del wifimanager
	esp_wifi_set_ps(WIFI_PS_NONE);
}

void Wifi::evento(void* arg, esp_event_base_t base, int32_t id, void* data){
	Wifi* self = (Wifi*) arg;
	if(base == WIFI_EVENT && id == WIFI_EVENT_STA_START){
		esp_wifi_connect();
	}else if(base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED){
		esp_wifi_connect();
	}else if(base == IP_EVENT && id == IP_EVENT_STA_GOT_IP){
		mdns();
		xEventGroupSetBits(self->e, WIFI_CONNECTED);
	}
}

void Wifi::espera(){
	xEventGroupWaitBits(e, WIFI_CONNECTED, pdFALSE, pdFALSE, portMAX_DELAY);
}

//metodo para inicializar el mDNS
void Wifi::mdns(){
	if (mdns_init() == ESP_OK) {
        mdns_hostname_set("robot-sumo");
        ESP_LOGI("MDNS", "Servicio iniciado: robot-sumo.local");
    } else {
        ESP_LOGE("MDNS", "Fallo al iniciar mDNS");
    }
}
