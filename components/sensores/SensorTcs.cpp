#include "SensorTcs.h"

#ifndef CONFIG_IDF_TARGET_ESP32S3

#include "Nvs.h"
#include "driver/i2c.h"
#include "rgb.h"
#include "eventos.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <cstdint>

#define TCSADDR 0x29 // Dirección I2C estándar del TCS34725
#define TAG "SensorTcs"

#define TCS_ENABLE 0x80 | 0x00
#define TCS_ATIME 0x80 | 0x01
#define TCS_CONTROL 0x80 | 0x0F
#define TCS_CDATAL 0x80 | 0x14

extern volatile bool start;

//constructor
SensorTcs::SensorTcs(int _limCol, GestorI2C* _i2c):
	//tolerancia de color
    limCol(_limCol),

    //variables de la logica
    estado(false),
    estado2(false),
	i2c(_i2c),

	//variables predeterminadas
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


bool SensorTcs::read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c){
	uint8_t write_buf[1] = {TCS_CDATAL};
	uint8_t read_buf[8];

	//definir donde empezar a escribir los datos leidos
	esp_err_t err = i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf, 1, pdMS_TO_TICKS(20));
    if (err != ESP_OK) {
		i2c->error();
        return false;
    }

	//leer datos del color
	err = i2c_master_read_from_device(i2c->port(), TCSADDR, read_buf, 8, pdMS_TO_TICKS(20));
    if (err != ESP_OK) {
		i2c->error();
        return false;
    }

	//conbina los bytes para transformarlos a valores en 16bits
	*c = (read_buf[1] << 8) | read_buf[0];
    *r = (read_buf[3] << 8) | read_buf[2];
    *g = (read_buf[5] << 8) | read_buf[4];
    *b = (read_buf[7] << 8) | read_buf[6];

	i2c->reset();
	return true;
}

void SensorTcs::calCol(){
 	// Acumuladores para promediar las lecturas
  	uint32_t t_r = 0;
  	uint32_t t_g = 0;
  	uint32_t t_b = 0;
	uint32_t t_c = 0;
  	uint16_t r, g, b, c;
  	const int NUMM = 15;

	//primero sensor
  	for (int i = 0; i < NUMM; i++) {
  	  	// Leer sensor 1
  	  	mu.sel(i2c->port(), 0);
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

	//segundo sensor
	t_r = 0;
	t_g = 0;
	t_b = 0;
	t_c = 0;
  	for (int i = 0; i < NUMM; i++) {
  		// Leer sensor 2
  		mu.sel(i2c->port(), 3);
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
}

void SensorTcs::begin(){
	nvsLeer();
	uint8_t write_buf[2];

    write_buf[0] = TCS_ATIME;
    write_buf[1] = 0xFF;
	uint8_t write_buf_gain[2] = {TCS_CONTROL, 0x01};
	uint8_t write_buf_enable[2] = {TCS_ENABLE, 0x03};

	// selecciona sc_1
	mu.sel(i2c->port(), 0);
	// verifica el funcionamiento de sc_1
	if (i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
        estado = true;
    } else {
	    estado = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_1 (TCS34725)");
		rgb(0, 1023);
	}

	mu.sel(i2c->port(), 3);

    // Intentamos escribir la configuración
    if (i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(i2c->port(), TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
	    estado2 = true;
	} else {
	    estado2 = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_2 (TCS34725)");
		rgb(0, 1023);
	}

	//se  calibra si por lo menos uno funciono
	if(estado || estado2){
		calCol();
	}
	//se crea la tarea de los sensores de color
  	tarea = xTaskCreateStaticPinnedToCore(senColor, "sensorColor", sizeof(stackTcs), this, 3, stackTcs, &tcbTcs, 1);
}

bool SensorTcs::sc_1Verify(){
	//variable que guarda el resultado
	bool res = false;
	//variable tempotales
	uint16_t rt, gt, bt, ct;
    // detecta si el sensor de color funciona bien
    if (estado) {
    	// selecciona sc_1
    	mu.sel(i2c->port(), 0);
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
		}
	}
	vTaskDelay(1);
	return res;
}

bool SensorTcs::sc_2Verify(){
	//variable del resultado
	bool res = false;
	//variables temporales
	uint16_t rt, gt, bt, ct;
	if (estado2) {
      	// selecciona sc_2
      	mu.sel(i2c->port(), 3);
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
	vTaskDelay(1);
	return res;
}

//metodo que procesa las detecciones tanto de sc_1 como sc_2
void SensorTcs::procesar(){
	//se verifica si se detecta el limite en direccion A
	if(sc_1Verify()){
		//manda bit si sc_1 detecta el limite
		xEventGroupSetBits(eventos, BIT_LIM_A);
	}else{
		//limpia el bit si sc_1 deja de detectar el limite
		xEventGroupClearBits(eventos, BIT_LIM_A);
	}

	//se verifica si se detecta el limite en direccion B
	if(sc_2Verify()){
		//manda bit si sc_2 detecta el limite
		xEventGroupSetBits(eventos, BIT_LIM_B);
	}else{
		//limpia el bit si sc_2 deja de detectar el limite
		xEventGroupClearBits(eventos, BIT_LIM_B);
	}
}

//tarea que se encarga de procesar los datos de los sensores de color
void SensorTcs::senColor(void *pvParameters) {
	//se obtiene el puntero del objeto
	SensorTcs* sensor = (SensorTcs*)pvParameters;
	while (true) {
		//inicia el combate
		if (start) {
			//se verifican ambos sensores de color
			sensor->procesar();
		}
   	 	vTaskDelay(pdMS_TO_TICKS(10));
  	}
}

//metodo que lee la Nvs
void SensorTcs::nvsLeer(){
	//se accede al namspace de sensores
	Nvs nvs("sensores");
	//extrae el valor del limite de color
	limCol = nvs.leer("umbral_color", limCol);
}

//metdo que recopila las lecturas y datos en general para la telemetria
void SensorTcs::colores(uint16_t* buffer){
	//se envian los respecivos datos a la telemetria

	//Calibracion de sc_1
	buffer[0] = lcr;
	buffer[1] = lcg;
	buffer[2] = lcb;
	buffer[3] = lcc;

	//Calibracion de sc_2
	buffer[4] = lcr2;
	buffer[5] = lcg2;
	buffer[6] = lcb2;
	buffer[7] = lcc2;

	//lecturas de sc_1
	buffer[8] = r1;
	buffer[9] = g1;
	buffer[10] = b1;
	buffer[11] = c1;

	//lecturas de sc_2
	buffer[12] = r2;
	buffer[13] = g2;
	buffer[14] = b2;
	buffer[15] = c2;
}

#endif // !CONFIG_IDF_TARGET_ESP32S3
