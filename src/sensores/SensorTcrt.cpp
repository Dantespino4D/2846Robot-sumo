#include "SensorTcrt.h"
#include "../configuracion/eventos.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <cstdint>

extern EventGroupHandle_t eventos;

static const char* TAG = "SensorTcrt";

// Definición del miembro estático
TaskHandle_t SensorTcrt::tarea = nullptr;

SensorTcrt::SensorTcrt(gpio_num_t _p1, gpio_num_t _p2) :
	pin1(_p1),
	pin2(_p2)
{}

void IRAM_ATTR SensorTcrt::limite_isr(void* arg) {
	//variable para determinar si se debe cambiar de contexto
    BaseType_t cambioC = pdFALSE;

	//se notifica a la tarea de TCRT que se ha producido un cambio
	vTaskNotifyGiveFromISR(tarea, &cambioC);

	//si se ha notificado a la tarea, se solicita un cambio de contexto
    if (cambioC) {
        portYIELD_FROM_ISR();
    }
}

void SensorTcrt::begin() {
	//inicializa los pines con la configuración de las interrupciones
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin1) | (1ULL << pin2);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

	//instala el servicio de interrupciones
    gpio_install_isr_service(0);

	//agrega las interrupciones para ambos pines
    gpio_isr_handler_add(pin1, &SensorTcrt::limite_isr, (void*)pin1);
    gpio_isr_handler_add(pin2, &SensorTcrt::limite_isr, (void*)pin2);

    ESP_LOGI(TAG, "Interrupciones configuradas para TCRT en pines %d y %d", pin1, pin2);

	//crea la tarea para procesar las interrupciones
	xTaskCreate(tareaTcrt, "tareaTcrt", 2048, (void*)this, 10, &tarea);
}


void SensorTcrt::tareaTcrt(void* pvParameters) {
	SensorTcrt* sensor = (SensorTcrt*)pvParameters;
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(sensor == nullptr) {
			ESP_LOGE(TAG, "Error: puntero a SensorTcrt es nulo");
			continue;
		}
		if(gpio_get_level(sensor->pin1)){
			xEventGroupSetBits(eventos, BIT_LIM_A);
		}else{
			xEventGroupClearBits(eventos, BIT_LIM_A);
		}
		if(gpio_get_level(sensor->pin2)){
			xEventGroupSetBits(eventos, BIT_LIM_B);
		}else{
			xEventGroupClearBits(eventos, BIT_LIM_B);
		}
	}
}
