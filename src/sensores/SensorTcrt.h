#ifndef SENSORTCRT_H
#define SENSORTCRT_H

#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "SensorLimite.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <cstdint>
#include "esp_attr.h"

class SensorTcrt : public SensorLimite {
	private:
		gpio_num_t pin1;
		gpio_num_t pin2;
		uint64_t tempU;

		static TaskHandle_t tarea;

		static void tareaTcrt(void* pvParameters);

		static void IRAM_ATTR limite_isr(void* arg);

	public:
		SensorTcrt(gpio_num_t _p1, gpio_num_t _p2);
		virtual ~SensorTcrt() {}
		void begin() override;
		void colores(uint16_t* buffer) override;
};

#endif // CONFIG_IDF_TARGET_ESP32S3
#endif // SENSORTCRT_H
