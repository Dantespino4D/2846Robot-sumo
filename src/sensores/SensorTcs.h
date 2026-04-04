#ifndef SENSORTCS_H
#define SENSORTCS_H

#include "driver/i2c.h"
#include "freertos/task.h"
#include "../actuadores/Multiplexor.h"
#include "SensorLimite.h"
#include <cstdint>

class SensorTcs : public SensorLimite {
	private:
		int limCol;
		bool estado;
		bool estado2;

		SemaphoreHandle_t* mutex;

		Multiplexor* mu;

		int redC;
		int green;
		int blue;

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
	public:
		SensorTcs(int limCol, Multiplexor* _mu, SemaphoreHandle_t* _mutex);
		virtual ~SensorTcs() {}

		void calCol();

		bool sc_1Verify();
		bool sc_2Verify();

		void procesar();

		void begin() override;

		void colores(uint16_t* rc, uint16_t* gc, uint16_t* bc, uint16_t* cc, uint16_t* rc2, uint16_t* gc2, uint16_t* bc2, uint16_t* cc2, uint16_t* red1, uint16_t* green1, uint16_t* blue1, uint16_t* clear1, uint16_t* red2, uint16_t* green2, uint16_t* blue2, uint16_t* clear2);
};
#endif
