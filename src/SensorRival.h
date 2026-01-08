#ifndef SENSORRIVAL_H
#define SENSORRIVAL_H


#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"
#include "driver/rmt_types.h"

#define N_MUESTRAS 5

class SensorRival{
	private:
		//pines de los sensores
		gpio_num_t trig_1;
		gpio_num_t trig_2;
		gpio_num_t echo_1;
		gpio_num_t echo_2;

		//handles del rmt
		rmt_channel_handle_t txC1;
		rmt_channel_handle_t txC2;
		rmt_channel_handle_t rxC1;
		rmt_channel_handle_t rxC2;

		//el handle del encoder
		rmt_encoder_handle_t encoder;

		//buffer de los datos recibidos por echo
		rmt_symbol_word_t buf[9];

		//vaiables de las distancias leidas
		uint16_t dis1;
		uint16_t dis2;

		//handle de la tarea
		TaskHandle_t task;

		//limite de distancia
		int maxd;

		//variables que almacenan muestas
		uint16_t mem1[N_MUESTRAS];
		uint16_t mem2[N_MUESTRAS];

		//variables que de los indices
		int ind1;
		int ind2;

		//variables de los totales promedio
		long total1;
		long total2;

		//metodo para medir distancia
		uint16_t dist_cm(gpio_num_t trig_pin, gpio_num_t echo_pin, rmt_channel_handle_t rxC, rmt_channel_handle_t txC);

		uint16_t filtro(gpio_num_t trig, gpio_num_t echo, rmt_channel_handle_t rxC, rmt_channel_handle_t txC, uint16_t* mem, int& ind, long& total);

		//metodo del callback
		static bool IRAM_ATTR evento(rmt_channel_handle_t rxC, const rmt_rx_done_event_data_t *data, void *user);
	public:
		//costructor
		SensorRival(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2);

		//inicializamos el RMT
		void begin();

		//metodo para leer los valores del nvs
		void nvsLeer();

		//metodos de verificacion
		bool ojos_1Verify();
		bool ojos_2Verify();

		//metodo para devolver las distancias
		void distancias(uint16_t* d1, uint16_t* d2);
};

#endif // !SENSORRIVAL_H
