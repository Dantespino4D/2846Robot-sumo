#include "SensorTcrt.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "../configuracion/eventos.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <cstdint>

extern EventGroupHandle_t eventos;

static const char* TAG = "SensorTcrt";

TaskHandle_t SensorTcrt::tarea = nullptr;

SensorTcrt::SensorTcrt(gpio_num_t _p1, gpio_num_t _p2) :
	pin1(_p1),
	pin2(_p2)
{}

void IRAM_ATTR SensorTcrt::limite_isr(void* arg) {
    BaseType_t cambioC = pdFALSE;
	vTaskNotifyGiveFromISR(tarea, &cambioC);
    if (cambioC) {
        portYIELD_FROM_ISR();
    }
}

void SensorTcrt::begin() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin1) | (1ULL << pin2);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Interrupciones configuradas para TCRT en pines %d y %d", pin1, pin2);

	xTaskCreatePinnedToCore(tareaTcrt, "tareaTcrt", 2048, (void*)this, 10, &tarea, 1);
}

void SensorTcrt::colores(uint16_t* buffer) {
    buffer[0] = (uint16_t)gpio_get_level(pin1);
    buffer[1] = (uint16_t)gpio_get_level(pin2);
    for(int i = 2; i < 16; i++) {
        buffer[i] = 0;
    }
}

void SensorTcrt::tareaTcrt(void* pvParameters) {
	SensorTcrt* sensor = (SensorTcrt*)pvParameters;

	//instalar la interrupcion
    gpio_install_isr_service(0);

    gpio_isr_handler_add(sensor->pin1, &SensorTcrt::limite_isr, (void*)sensor->pin1);
    gpio_isr_handler_add(sensor->pin2, &SensorTcrt::limite_isr, (void*)sensor->pin2);
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

#endif // CONFIG_IDF_TARGET_ESP32S3
