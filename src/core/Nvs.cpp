#include "Nvs.h"
#include "nvs.h"
#include <cstdint>


Nvs::Nvs(const char* _nameE){
	nameE = _nameE;
}


void Nvs::guardar(const char* clave, int32_t valor){
	//se declara el handle
	nvs_handle_t nvs;

	esp_err_t err = nvs_open(nameE, NVS_READWRITE, &nvs);
	if(err == ESP_OK){
		nvs_set_i32(nvs, clave, valor);
		nvs_commit(nvs);
		nvs_close(nvs);
	}
}

int32_t Nvs::leer(const char* clave, int32_t valor){
	//se declara el handle
	nvs_handle_t nvs;

	esp_err_t err = nvs_open(nameE, NVS_READWRITE, &nvs);
	if(err == ESP_OK){
		nvs_get_i32(nvs, clave, &valor);
		nvs_close(nvs);
	}
	return valor;
}
