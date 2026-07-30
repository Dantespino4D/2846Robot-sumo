#include "GestorBorde.h"
#include "MaquinaEstados.h"
#include "pines.h"
#include "driver/gpio.h"

#define TAG "GestorBorde"

GestorBorde::GestorBorde() {
	// Constructor implementation
}

void GestorBorde::begin(MaquinaEstados* me) {
	//se inicializa el pin de interrupcion
	gpio_config_t io_stm = {
		.pin_bit_mask = (1ULL << STM32_INT),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_ANYEDGE
	};
	esp_err_t err = gpio_config(&io_stm);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error configuring GPIO: %d", err);
	}

	err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error installing ISR service: %d", err);
	}

	gpio_isr_handler_add(STM32_INT, INT_Borde, me);
}

void IRAM_ATTR GestorBorde::INT_Borde(void* arg) {
	MaquinaEstados* me = static_cast<MaquinaEstados*>(arg);
	if(me != nullptr) {
		me->definirEvasion(gpio_get_level(STM32_INT) == 1);
	}
}
