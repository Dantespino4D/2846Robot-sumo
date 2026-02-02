#include "SensorLimite.h"
#include "Nvs.h"
#include "driver/i2c.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "rgb.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "rgb.h"
#include "esp_rom_sys.h"
#include "rom/ets_sys.h"
#include <cstdint>

#define TCSADDR 0x29 // Dirección I2C estándar del TCS34725
#define TAG "SensorLimite"

#define TCS_ENABLE 0x80 | 0x00
#define TCS_ATIME 0x80 | 0x01
#define TCS_CONTROL 0x80 | 0x0F
#define TCS_CDATAL 0x80 | 0x14

//constructor
SensorLimite::SensorLimite(int _limCol, Multiplexor* _mu, SemaphoreHandle_t* _mutex):
	//tolerancia de color
    limCol(_limCol),

    //variables de la logica
    estado(false),
    estado2(false),
	mutex(_mutex),
	mu(_mu),

	//variables predeterminadas
    redC(50),
    green(50),
    blue(50),
    lcr(50),
	lcg(50),
    lcb(50),
	lcc(50),
	lcr2(50),
	lcg2(50),
	lcb2(50),
	lcc2(50),
	r1(0),
	g1(0),
	b1(0),
	c1(0),
	r2(0),
	g2(0),
	b2(0),
	c2(0)
{}


bool SensorLimite::read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c){
	uint8_t write_buf[1] = {TCS_CDATAL};
	uint8_t read_buf[8];

	//definir donde empezar a escribir los datos leidos
	esp_err_t err = i2c_master_write_to_device(mu->port(), TCSADDR, write_buf, 1, pdMS_TO_TICKS(20));
    if (err != ESP_OK) {
		mu->error();
        return false;
    }

	//leer datos del color
	err = i2c_master_read_from_device(mu->port(), TCSADDR, read_buf, 8, pdMS_TO_TICKS(20));
    if (err != ESP_OK) {
		mu->error();
        return false;
    }

	//conbina los bytes para transformarlos a valores en 16bits
	*c = (read_buf[1] << 8) | read_buf[0];
    *r = (read_buf[3] << 8) | read_buf[2];
    *g = (read_buf[5] << 8) | read_buf[4];
    *b = (read_buf[7] << 8) | read_buf[6];

	mu->reset();
	return true;
}

void SensorLimite::calCol(){
 	// Acumuladores para promediar las lecturas
  	uint32_t t_r = 0;
  	uint32_t t_g = 0;
  	uint32_t t_b = 0;
	uint32_t t_c = 0;
  	uint16_t r, g, b, c;
  	const int NUMM = 15;

	//se detecta recibe el mutex
	if(xSemaphoreTake(*mutex, pdMS_TO_TICKS(50)) == pdTRUE){
		//primero sensor
  		for (int i = 0; i < NUMM; i++) {
  		  	// Leer sensor 1
  		  	mu->sel(0);
  		  	if(read(&r, &g, &b, &c)){
				t_r += r;
  		  		t_g += g;
  		  		t_b += b;
				t_c += c;
			}
		    vTaskDelay(pdMS_TO_TICKS(10));
  		}
		// calcula el promedio de las muestras
 		lcr = t_r / NUMM;
		lcg = t_g / NUMM;
  		lcb = t_b / NUMM;
		lcc = t_c / NUMM;

		ESP_LOGI(TAG, "Calibracion: R=%d, G=%d, B=%d, C=%d", lcr, lcg, lcb, lcc);

		//se libera el mutex
		xSemaphoreGive(*mutex);
	}else{
		rgb(0, 1023);
	}
	//segundo sensor
	t_r = 0;
	t_g = 0;
	t_b = 0;
	t_c = 0;
	if(xSemaphoreTake(*mutex, pdMS_TO_TICKS(50)) == pdTRUE){
  		for (int i = 0; i < NUMM; i++) {
  	  		// Leer sensor 2
  	  		mu->sel(3);
  	  		if(read(&r, &g, &b, &c)){
				t_r += r;
  	  			t_g += g;
  	  			t_b += b;
				t_c += c;
			}
	    	vTaskDelay(pdMS_TO_TICKS(10));
  		}
  		// calcula el promedio de las muestras
 		lcr2 = t_r / NUMM;
		lcg2 = t_g / NUMM;
  		lcb2 = t_b / NUMM;
		lcc2 = t_c / NUMM;

		ESP_LOGI(TAG, "Calibracion 2: R=%d, G=%d, B=%d, C=%d", lcr2, lcg2, lcb2, lcc2);

		//se libera el mutex
		xSemaphoreGive(*mutex);
	}
	else{
		rgb(0, 1023);
	}
}

void SensorLimite::begin(){
	nvsLeer();
	uint8_t write_buf[2];

    write_buf[0] = TCS_ATIME;
    write_buf[1] = 0xFF;
	uint8_t write_buf_gain[2] = {TCS_CONTROL, 0x01};
	uint8_t write_buf_enable[2] = {TCS_ENABLE, 0x03};

	// selecciona sc_1
	mu->sel(0);
	// verifica el funcionamiento de sc_1
	if (i2c_master_write_to_device(mu->port(), TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(mu->port(), TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(mu->port(), TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
        estado = true;
    } else {
	    estado = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_1 (TCS34725)");
		rgb(0, 1023);
	}

	mu->sel(3);


    // Intentamos escribir la configuración
    if (i2c_master_write_to_device(mu->port(), TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(mu->port(), TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(mu->port(), TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
	    estado2 = true;
	} else {
	    estado2 = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_2 (TCS34725)");
		rgb(0, 1023);
	}
}

bool SensorLimite::sc_1Verify(){
	//variable que guarda el resultado
	bool res = false;
	//variable tempotales
	uint16_t rt, gt, bt, ct;
	if(xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE){
    	// detecta si el sensor de color funciona bien
    	if (estado) {
    		// selecciona sc_1
    	  	mu->sel(0);
      		// sc_1 lee el color
      		if(read(&rt, &gt, &bt, &ct)){
				//se aplican
				r1 = rt;
				g1 = gt;
				b1 = bt;
				c1 = ct;
      			// sc_1 determina si el color detectado es el mismo del limite
      			long difCol = labs(r1 - lcr) + labs(g1 - lcg) + labs(b1 - lcb);
      			if (difCol > limCol) {
        			// retorna verdadero al detectar el limite
					res = true;
				}
			}else{
				rgb(0, 1023);
			}
		}
		xSemaphoreGive(*mutex);
	}
	vTaskDelay(1);
	return res;
}

bool SensorLimite::sc_2Verify(){
	//variable del resultado
	bool res = false;
	//variables temporales
	uint16_t rt, gt, bt, ct;
	if(xSemaphoreTake(*mutex, portMAX_DELAY) == pdTRUE){
		if (estado2) {
      		// selecciona sc_2
      		mu->sel(3);
      		// sc_2 lee el color
      		if(read(&rt, &gt, &bt, &ct)){
				r2 = rt;
				g2 = gt;
				b2 = bt;
				c2 = ct;
      			// sc_2 determina si el color detectado es el mismo del limite
      			long difCol = labs(r2 - lcr2) + labs(g2 - lcg2) + labs(b2 - lcb2);
      			if (difCol > limCol) {
        			// retorna verdadero al detectar el limite
					res = true;
				}
			}
    	}
		else{
			rgb(0, 1023);
		}
		xSemaphoreGive(*mutex);
	}
	vTaskDelay(1);
	return res;
}

void SensorLimite::nvsLeer(){
	Nvs nvs("sensores");
	limCol = nvs.leer("umbral_color", limCol);
}

void SensorLimite::colores(uint16_t* rc, uint16_t* gc, uint16_t* bc, uint16_t* cc, uint16_t* rc2, uint16_t* gc2, uint16_t* bc2, uint16_t* cc2, uint16_t* red1, uint16_t* green1, uint16_t* blue1, uint16_t* clear1, uint16_t* red2, uint16_t* green2, uint16_t* blue2, uint16_t* clear2){
	//mutex para evitar datos correptos
	if(xSemaphoreTake(*mutex, 0) == pdTRUE){
		//se envian los respecivos datos a la telemetria
		*rc = lcr;
		*gc = lcg;
		*bc = lcb;
		*cc = lcc;
		*rc2 = lcr2;
		*gc2 = lcg2;
		*bc2 = lcb2;
		*cc2 = lcc2;

		*red1 = r1;
		*green1 = g1;
		*blue1 = b1;
		*clear1 = c1;
		*red2 = r2;
		*green2 = g2;
		*blue2 = b2;
		*clear2 = c2;
		//se suelta el mutex
		xSemaphoreGive(*mutex);
	}else{
		//si no se puede leer envia in valor irreal a la telemtetria
		uint16_t eVal = 65535;
		*rc = eVal;
		*gc = eVal;
		*bc = eVal;
		*cc = eVal;
		*rc2 = eVal;
		*gc2 = eVal;
		*bc2 = eVal;
		*cc2 = eVal;

		*red1 = eVal;
		*green1 = eVal;
		*blue1 = eVal;
		*clear1 = eVal;
		*red2 = eVal;
		*green2 = eVal;
		*blue2 = eVal;
		*clear2 = eVal;
	}
}
