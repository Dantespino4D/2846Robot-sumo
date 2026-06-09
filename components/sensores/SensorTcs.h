#ifndef SENSORTCS_H
#define SENSORTCS_H

#include "sdkconfig.h"

#ifndef CONFIG_IDF_TARGET_ESP32S3

#include "driver/i2c_master.h"
#include "freertos/task.h"
#include "GestorI2C.h"
#include "Multiplexor.h"
#include "SensorLimite.h"
#include <cstdint>

class SensorTcs : public SensorLimite {
	private:
		int limCol;
		bool estado;
		bool estado2;

		GestorI2C* i2c;
		Multiplexor mu;

		uint16_t lcr, lcg, lcb, lcc;
		uint16_t lcr2, lcg2, lcb2, lcc2;

		bool read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
		void nvsLeer();

		uint16_t r1;
		uint16_t g1;
		uint16_t b1;
		uint16_t c1;
		uint16_t r2;
		uint16_t g2;
		uint16_t b2;
		uint16_t c2;

		//handle de la tarea
		TaskHandle_t tarea = NULL;
		//tcb de la tarea
		StaticTask_t tcbTcs;
		//stack de la tarea
		StackType_t stackTcs[2048];
	public:
		SensorTcs(int limCol, GestorI2C* _i2c);
		virtual ~SensorTcs() {}

		void calCol();

		bool sc_1Verify();
		bool sc_2Verify();

		static void senColor(void* pvParameters);

		void procesar();

		void begin() override;

		void colores(uint16_t* buffer) override;
};

#endif // !CONFIG_IDF_TARGET_ESP32S3
#endif // SENSORTCS_H
