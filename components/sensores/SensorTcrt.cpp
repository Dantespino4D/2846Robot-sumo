#include "SensorTcrt.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "eventos.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <cstdint>
#include "esp_attr.h"

extern EventGroupHandle_t eventos;

static const char* TAG = "SensorTcrt";

SensorTcrt::SensorTcrt(gpio_num_t _p1, gpio_num_t _p2, gpio_num_t _p3, gpio_num_t _p4) :
	pin1(_p1),
	pin2(_p2),
	pin3(_p3),
	pin4(_p4)
{}

void IRAM_ATTR SensorTcrt::limite_isr(void* arg) {
	SensorTcrt* sensor = static_cast<SensorTcrt*>(arg);
	uint32_t estado = 0;
	BaseType_t cambioC = pdFALSE;

	if (sensor == nullptr || sensor->tarea == NULL) {
		return;
	}

	if(gpio_get_level(sensor->pin1)){
		estado |= (1 << 0);
	}
	if(gpio_get_level(sensor->pin2)){
		estado |= (1 << 1);
	}
	if(gpio_get_level(sensor->pin3)){
		estado |= (1 << 2);
	}
	if(gpio_get_level(sensor->pin4)){
		estado |= (1 << 3);
	}
	xTaskNotifyFromISR(sensor->tarea, estado, eSetValueWithOverwrite, &cambioC);
    if (cambioC) {
        portYIELD_FROM_ISR();
    }
}

void SensorTcrt::begin() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin1) | (1ULL << pin2) | (1ULL << pin3) | (1ULL << pin4);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Interrupciones configuradas para 4 TCRT en pines %d, %d, %d y %d", pin1, pin2, pin3, pin4);

	tarea = xTaskCreateStaticPinnedToCore(tareaTcrt, "tareaTcrt", sizeof(stackTcrt), (void*)this, 10, stackTcrt, &tcbTcrt, 1);
}

void SensorTcrt::colores(uint16_t* buffer) {
    buffer[0] = (uint16_t)gpio_get_level(pin1);
    buffer[1] = (uint16_t)gpio_get_level(pin2);
    buffer[2] = (uint16_t)gpio_get_level(pin3);
    buffer[3] = (uint16_t)gpio_get_level(pin4);
    for(int i = 4; i < 16; i++) {
        buffer[i] = 0;
    }
}

void SensorTcrt::tareaTcrt(void* pvParameters) {
	SensorTcrt* sensor = (SensorTcrt*)pvParameters;

	// instalar la interrupcion si no esta instalada
	esp_err_t isr_err = gpio_install_isr_service(0);
	if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) {
		ESP_LOGE(TAG, "Error instalando ISR service TCRT: %s", esp_err_to_name(isr_err));
	}

    gpio_isr_handler_add(sensor->pin1, &SensorTcrt::limite_isr, (void*)sensor);
    gpio_isr_handler_add(sensor->pin2, &SensorTcrt::limite_isr, (void*)sensor);
    gpio_isr_handler_add(sensor->pin3, &SensorTcrt::limite_isr, (void*)sensor);
    gpio_isr_handler_add(sensor->pin4, &SensorTcrt::limite_isr, (void*)sensor);

	uint32_t estado = 0;
	while (true) {
		if(xTaskNotifyWait(0, 0, &estado, portMAX_DELAY) == pdTRUE){
			if(sensor == nullptr) {
				ESP_LOGE(TAG, "Error: puntero a SensorTcrt es nulo");
				continue;
			}
			
			// Procesar bits de sensores TCRT
			if(estado & (1 << 0)) xEventGroupSetBits(eventos, BIT_LIM_AI);
			else xEventGroupClearBits(eventos, BIT_LIM_AI);

			if(estado & (1 << 1)) xEventGroupSetBits(eventos, BIT_LIM_AD);
			else xEventGroupClearBits(eventos, BIT_LIM_AD);

			if(estado & (1 << 2)) xEventGroupSetBits(eventos, BIT_LIM_BI);
			else xEventGroupClearBits(eventos, BIT_LIM_BI);

			if(estado & (1 << 3)) xEventGroupSetBits(eventos, BIT_LIM_BD);
			else xEventGroupClearBits(eventos, BIT_LIM_BD);
		}
	}
}

#endif // CONFIG_IDF_TARGET_ESP32S3
