#include "Wifi.h"
#include "freertos/FreeRTOS.h"
#include "esp_bit_defs.h"
#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include <cstdint>
#include <cstring>
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "mdns.h"
#include "esp_smartconfig.h"
#include "../core/Nvs.h"
#include "esp_system.h"

const char* TAG = "wifi";

void Wifi::begin(){
	//crear un grupo de eventos
	e = xEventGroupCreate();

	esp_netif_create_default_wifi_sta();

	intentos = 0;

	wifi();
}

void Wifi::wifi(){
	//reservar la memoria
	wifi_init_config_t conf = WIFI_INIT_CONFIG_DEFAULT();

	//verififcar si hay algun error
	ESP_ERROR_CHECK(esp_wifi_init(&conf));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::evento, this, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi::evento, this, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(SC_EVENT, ESP_EVENT_ANY_ID, &Wifi::evento, this, NULL));

	//se establece en modo sta
	esp_wifi_set_mode(WIFI_MODE_STA);
	wifi_config_t w = {};
    strcpy((char*)w.sta.ssid, WIFI);
    strcpy((char*)w.sta.password, WIFI_PASS_CRED);
    w.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &w));
	esp_wifi_start();
	//mobjeto del wifimanager
	esp_wifi_set_ps(WIFI_PS_NONE);
}

void Wifi::evento(void* arg, esp_event_base_t base, int32_t id, void* data){
	Wifi* self = (Wifi*) arg;
	if(base == WIFI_EVENT && id == WIFI_EVENT_STA_START){
    	// INTENTAR CONECTAR Y VERIFICAR SI FALLA INMEDIATAMENTE
    	esp_err_t err = esp_wifi_connect();
    	if (err == ESP_ERR_WIFI_SSID) {
        	// Si no hay SSID guardado, ir directo a SmartConfig
        	ESP_LOGW("WIFI", "No hay credenciales guardadas. Iniciando SmartConfig...");
        	self->smart();
    	}
	}else if(base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED){
		if(self->intentos < 5){
			esp_wifi_connect();
			self->intentos++;
		}else{
			self->smart();
		}
	}else if(base == IP_EVENT && id == IP_EVENT_STA_GOT_IP){
		self->intentos = 0;
		self->mdns();
		xEventGroupSetBits(self->e, WIFI_CONNECTED);
	}else if(base == SC_EVENT && id == SC_EVENT_SCAN_DONE){

	}else if(base == SC_EVENT && id == SC_EVENT_FOUND_CHANNEL){

	}else if(base == SC_EVENT && id == SC_EVENT_GOT_SSID_PSWD){
		smartconfig_event_got_ssid_pswd_t* smartC = (smartconfig_event_got_ssid_pswd_t*) data;
		wifi_config_t wi = {
			.sta = {
				.ssid = 0,
				.password = 0,
				.threshold = {
					.authmode = WIFI_AUTH_WPA2_PSK,
				},
			}
		};
		memcpy(&wi.sta.ssid, smartC->ssid, sizeof(wi.sta.ssid));
		memcpy(&wi.sta.password, smartC->password, sizeof(wi.sta.password));

		wi.sta.bssid_set = smartC->bssid_set;
		if (wi.sta.bssid_set == true) {
	    	memcpy(wi.sta.bssid, smartC->bssid, sizeof(wi.sta.bssid));
		}

		esp_wifi_disconnect();
		esp_wifi_set_config(WIFI_IF_STA, &wi);
		esp_wifi_connect();
	}else if(base == SC_EVENT && id == SC_EVENT_SEND_ACK_DONE){
		esp_smartconfig_stop();

	}

}

void Wifi::espera(){
    ESP_LOGW(TAG, "ESPERA: El programa está pausado esperando conexión WiFi...");
	EventBits_t bits = xEventGroupWaitBits(e, WIFI_CONNECTED, pdFALSE, pdFALSE, pdMS_TO_TICKS(20000));
	if(bits & WIFI_CONNECTED){
    	ESP_LOGI(TAG, "ESPERA: WiFi conectado con éxito, continuando...");
	}else{
		ESP_LOGE(TAG, "TIMEOUT: No se pudo conectar al WiFi. Cambiando a modo combate y reiniciando...");
        Nvs sys("sistema");
        sys.guardar("modo", 1);
        esp_restart();
	}
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

//funcion que gestiona las contraseñas y redes para poder conectarse a multiples redes
void Wifi::smart(){
	//se elige el protocologo que va a usa
	esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
	//se establece la configuracion
	smartconfig_start_config_t con = SMARTCONFIG_START_CONFIG_DEFAULT();
	//inicializar smart config
	esp_err_t sm = esp_smartconfig_start(&con);
	if(sm == ESP_OK){
		ESP_LOGW("smartconfig", "========================================================");
		ESP_LOGW("smartconfig", "SMARTCONFIG ACTIVADO: El robot no tiene internet.");
		ESP_LOGW("smartconfig", "Por favor, usa la App 'ESPTouch' en tu celular para");
		ESP_LOGW("smartconfig", "enviar el nombre y contraseña de tu red WiFi.");
		ESP_LOGW("smartconfig", "========================================================");
	}
}

void Wifi::signalW(int* _signal){
	wifi_ap_record_t info;
	int signal = 0;
	if(esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
		signal = info.rssi;
	}
	*_signal = signal;
}
