#include "Ota.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>

static const char *TAG = "OTA";

void Ota::ota(const char* url){
	char *urlC = strdup(url);
	if(urlC != NULL){
		xTaskCreate(Ota::tareaOta, "tarea ota", 8192, (void*)urlC, 5, NULL);
		ESP_LOGI(TAG, "iniciando ota");
	}else{
		ESP_LOGE(TAG, "error al iniciar la ota");
	}
}

void Ota::tareaOta(void *pvParameter){
	char *url = (char*)pvParameter;
	esp_http_client_config_t confH = {
		.url = url,
		.cert_pem = NULL,
		.skip_cert_common_name_check = true,
		.keep_alive_enable = true
	};

	esp_https_ota_config_t confO = {
		.http_config = &confH,
	};

	esp_err_t err = esp_https_ota(&confO);

	if(err == ESP_OK){
		ESP_LOGI(TAG, "ota exitoso");
		free(url);
		esp_restart();
	}else{
		ESP_LOGE(TAG, "OTA falló: %s", esp_err_to_name(err));
		free(url);
		vTaskDelete(NULL);
	}
}
