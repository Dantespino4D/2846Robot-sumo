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
		size_t len = strlen(urlC);
        while(len > 0 && (isspace((unsigned char)urlC[len - 1]) || urlC[len - 1] == '\r' || urlC[len - 1] == '\n')) {
            urlC[len - 1] = '\0';
            len--;
        }

        // 2. Eliminar comillas dobles al final (SI EXISTEN)
        if(len > 0 && urlC[len - 1] == '\"') {
            urlC[len - 1] = '\0';
            len--;
        }

        char* urlF = urlC;

        // 3. Eliminar espacios al inicio
        while(*urlF && isspace((unsigned char)*urlF)) {
            urlF++;
        }

        // 4. Eliminar comillas dobles al inicio (SI EXISTEN)
        if(*urlF == '\"') {
            urlF++;
        }

		char *urlT = strdup(urlF);

        // Log para ver EXACTAMENTE qué estamos enviando (entre corchetes)
        ESP_LOGI(TAG, "URL Limpia: [%s]", urlT);
		ESP_LOGI(TAG, "iniciando ota");
		xTaskCreate(Ota::tareaOta, "tarea ota", 8192, (void*)urlT, 5, NULL);
	}else{
		ESP_LOGE(TAG, "error al iniciar la ota");
	}
}

void Ota::tareaOta(void *pvParameter){
	char *url = (char*)pvParameter;
	esp_http_client_config_t confH = {};
	confH.url = url;
	confH.cert_pem = NULL;
	confH.crt_bundle_attach = NULL;
	confH.keep_alive_enable = true;
	confH.skip_cert_common_name_check = true;

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
