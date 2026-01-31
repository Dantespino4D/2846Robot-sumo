#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "SensorUltra.h"
#include "Nvs.h"
#include "driver/gpio.h"
#include "freertos/projdefs.h"
#include "rom/ets_sys.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"
#include "driver/rmt_types.h"
#include "esp_log.h" // Added for logging
#include <cstdint>

SensorUltra::SensorUltra(int _maxd, gpio_num_t _trig_1, gpio_num_t _echo_1, gpio_num_t _trig_2, gpio_num_t _echo_2):
	trig_1(_trig_1),
	trig_2(_trig_2),
	echo_1(_echo_1),
	echo_2(_echo_2),
	txC1(NULL),
	txC2(NULL),
	rxC1(NULL),
	rxC2(NULL),
	encoder(NULL),
	dis1(0),
	dis2(0),
    maxd(_maxd),
	ind1(0),
	ind2(0),
	total1(0),
	total2(0)
{
for(int i = 0; i < N_MUESTRAS; i++) {
        mem1[i] = 0;
        mem2[i] = 0;
    }
}

//metodo que inicializa cosas
bool SensorUltra::begin(){
	nvsLeer();
	//creamos el encoder
	rmt_copy_encoder_config_t confE = {};
	ESP_ERROR_CHECK(rmt_new_copy_encoder(&confE, &encoder));

	//configuracion de canal tx1
	rmt_tx_channel_config_t confT1 = {
		.gpio_num = trig_1,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = 64,
		.trans_queue_depth = 4,
	};
	//handle del canal tx1
	ESP_ERROR_CHECK(rmt_new_tx_channel(&confT1, &txC1));

	//configuracion de canal tx2
	rmt_tx_channel_config_t confT2 = {
		.gpio_num = trig_2,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = 64,
		.trans_queue_depth = 4,
	};
	//handle del canal tx2
	ESP_ERROR_CHECK(rmt_new_tx_channel(&confT2, &txC2));

	//configuracion del canal rx1
	rmt_rx_channel_config_t confR1 = {
		.gpio_num = echo_1,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = 64,
	};
	//handle del canal rx1
	ESP_ERROR_CHECK(rmt_new_rx_channel(&confR1, &rxC1));

	//configuracion del canal rx2
	rmt_rx_channel_config_t confR2 = {
		.gpio_num = echo_2,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = 64,
	};
	//handle del canal rx2
	ESP_ERROR_CHECK(rmt_new_rx_channel(&confR2, &rxC2));

	//se registrar los callback
	rmt_rx_event_callbacks_t cb = {
		.on_recv_done = evento,
	};

	ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rxC1, &cb, this));
	ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rxC2, &cb, this));

	//habilitamos los canales
	ESP_ERROR_CHECK(rmt_enable(txC1));
	ESP_ERROR_CHECK(rmt_enable(txC2));
	ESP_ERROR_CHECK(rmt_enable(rxC1));
	ESP_ERROR_CHECK(rmt_enable(rxC2));
	return true;
}

uint16_t SensorUltra::dist_mm(gpio_num_t trig, gpio_num_t echo, rmt_channel_handle_t rxC, rmt_channel_handle_t txC){
	rmt_symbol_word_t pul[3];

	//se guarda el handle
	this->task = xTaskGetCurrentTaskHandle();

	//configuracion inicial del payload
	pul[0].level0 = 1;
	pul[0].duration0 = 10;
	pul[0].level1 = 0;
	pul[0].duration1 = 0;

	//cofiguracion de la transmision
	rmt_transmit_config_t confT = {
		.loop_count = 0,
	};

	//configuracion de la recepcion dl pulso
	rmt_receive_config_t confR = {
		.signal_range_min_ns = 1000,
		.signal_range_max_ns = 25000000,
	};
	//detectar por echo (ponemos a escuchar ANTES de disparar)
	esp_err_t err = rmt_receive(rxC, buf, sizeof(buf), &confR);
	if(err != ESP_OK){
		if(err == ESP_ERR_INVALID_STATE){
			ESP_LOGE("SENSOR", "Error RMT: Canal RX no habilitado");
			rmt_disable(rxC);
			vTaskDelay(1);
			rmt_enable(rxC);
		}
		return 0;
	}

	//pulso trigger
	err = rmt_transmit(txC, encoder, pul, sizeof(pul), &confT);
	if(err != ESP_OK){
		rmt_disable(rxC);
		vTaskDelay(1);
		rmt_enable(rxC);
		return 0;
	}

	//espera los datos del callback
	if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5)) == 0){
		rmt_disable(rxC);
        vTaskDelay(1);
		rmt_enable(rxC);
        return 0;
    }

    //Retornamos el valor correspondiente al canal que acabamos de usar
    if(rxC == rxC1) {
		return dis1;
	}else if(rxC == rxC2) {
		return dis2;
	}
    return 0;
}

uint16_t SensorUltra::filtro(gpio_num_t trig, gpio_num_t echo, rmt_channel_handle_t rxC, rmt_channel_handle_t txC, uint16_t* mem, int& ind, long& total){
	//se eliminaba del total la lectura mas vieja
	total = total - mem[ind];

	//se le la nueva lectura
	int lec = dist_mm(trig, echo, rxC, txC);

	//se agrega a la lista
	mem[ind] = lec;

	//se agrega al total
	total = total + mem[ind];

	//se suma 1 al indice o lo restablecemos a 0 si llega al maximo
	ind++;
	if(ind >= N_MUESTRAS){
		ind = 0;
	}

	//retorna el promedio
	return total / N_MUESTRAS;
}

//metodo que procesara los eventos del callback
bool IRAM_ATTR SensorUltra::evento(rmt_channel_handle_t rxC, const rmt_rx_done_event_data_t *data, void *user){
	BaseType_t high_task_wakeup = pdFALSE;
	SensorUltra *self = (SensorUltra *)user;
	rmt_symbol_word_t *sim = (rmt_symbol_word_t *)data->received_symbols;
	int tiempo = 0;
	//extraemos el tiempo
	for(size_t i = 0; i < data->num_symbols; i++){
		if(sim[i].level0 == 1){
			tiempo = sim[i].duration0;
            break;
		}
	}
	uint16_t dis = (tiempo * 10) / 58;
	//calculamos la distancia
	if(tiempo > 0){
		if(rxC == self->rxC1){
			self->dis1 = dis;
		}else if(rxC == self->rxC2){
			self->dis2 = dis;
		}
	}
	vTaskNotifyGiveFromISR(self->task, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

//metodo que verifica ojos 1
bool SensorUltra::ojos_1Verify(){
	dis1 = filtro(trig_1, echo_1, rxC1, txC1, mem1, ind1, total1);
	return (dis1 > 0 && dis1 <= maxd);
}

//metodo que verifica ojos 2
bool SensorUltra::ojos_2Verify(){
	dis2 = filtro(trig_2, echo_2, rxC2, txC2, mem2, ind2, total2);
	return (dis2 > 0 && dis2 <= maxd);
}

void SensorUltra::procesar(TaskHandle_t* Robot){
	uint32_t pac = 0;
	if(ojos_1Verify()){
		pac |=(1 << 2);
	}
	/*if(ojos_2Verify()){
		pac |= (1 << 3);
	}*/
	if(pac > 0){
		xTaskNotify(*Robot, pac, eSetBits);
	}
}

//metodo que lee de la nvs la distancia maxima guardada
void SensorUltra::nvsLeer(){
	Nvs nvs("sensores");
	maxd = nvs.leer("dist_max", maxd);
}

//metodo que da las distancias para telemetria
void SensorUltra::getDistancias(uint16_t* buffer){
    buffer[0] = dis1;
    buffer[1] = dis2;
    // Rellenamos el resto (reservado para ToF) con 0
    for(int i=2; i<8; i++) {
        buffer[i] = 0;
    }
}
