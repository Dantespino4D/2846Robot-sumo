#include "SensorLimite.h"
#include "esp_log.h"
#include "freertos/task.h"

//pines ic2
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

// direccion del multiplexor
#define TCAADDR 0x70
#define TAG "SensorLimite"

//constructor
SensorLimite::SensorLimite(int _limCol):
	//tolerancia de color
    limCol(_limCol),

    //variables de la logica
    estado(false),
    estado2(false),

	//variables predeterminadas
    redC(50),
    green(50),
    blue(50),
    lcr(50),
	lcg(50),
    lcb(50),
{}

void SensorLimite::i2c(){
	//configuracion del i2c
	i2c_config_t conf = {};
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_MASTER_SDA_IO;
	conf.scl_io_num = I2C_MASTER_SCL_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	i2c_param_config(I2C_MASTER_NUM, &conf);

	esp_err_t err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	if (err != ESP_OK){
		ESP_LOGE(TAG, "fallo al instalar i2c %s", esp_err_to_name(err));
	}
}
void SensorLimite::scSel(uint8_t i){
	if(i > 7){
	return;
	}
	//selecciona el sensor de color a utilizar
    uint8_t data = 1 << i;
	esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, TCAADDR, &data, 1, pdMS_TO_TICKS(1000));
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al seleccionar canal del MUX I2C: %s", esp_err_to_name(err));
    }
}}

void SensorLimite::calCol(){
 	// Acumuladores para promediar las lecturas
  	uint32_t t_r = 0;
  	uint32_t t_g = 0;
  	uint32_t t_b = 0;

  	uint16_t r, g, b, c;
  	const int NUMM = 15;
  	for (int i = 0; i < NUMM; i++) {
  	  	// Leer sensor 1
  	  	scSel(0);
		vTaskDelay(pdMS_TO_TICKS(2));
  	  	sc_1.getRawData(&r, &g, &b, &c);
  	  	t_r += r;
  	  	t_g += g;
  	  	t_b += b;

	    vTaskDelay(pdMS_TO_TICKS(10));
  	}
  	// calcula el promedio de las muestras
 	lcr = t_r / NUMM;
	lcg = t_g / NUMM;
  	lcb = t_b / NUMM;
}

void SensorLimite::begin(){
	i2c();

	// selecciona sc_1
	scSel(0);
	vTaskDelay(pdMS_TO_TICKS(2));
	// verifica el funcionamiento de sc_1
	if (sc_1.begin()) {
		// todo bien
	    estado = true;
	} else {
	    // no funciona y desantiva su funcionamiento
    	estado = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_1 (TCS34725)");
	}

	// selecciona sc_2
	scSel(3);
	vTaskDelay(pdMS_TO_TICKS(2));
	// verifica el funcionamiento de sc_2
	if (sc_2.begin()) {
	    // todo bien
	    estado2 = true;
	} else {
		// no funciona y desantiva su funcionamiento
	    estado2 = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_2 (TCS34725)");
	}
}

bool SensorLimite::sc_1Verify(){
	// variables de los colores detectados
    uint16_t r, g, b, c;
    // detecta si el sensor de color funciona bien
    if (estado) {
    	// selecciona sc_1
      	scSel(0);
		vTaskDelay(pdMS_TO_TICKS(2));
      	// sc_1 lee el color
      	sc_1.getRawData(&r, &g, &b, &c);
      	// sc_1 determina si el color detectado es el mismo del limite
      	long difCol = abs(r - lcr) + abs(g - lcg) + abs(b - lcb);
      	if (difCol > limCol) {
        	// retorna verdadero al detectar el limite
			return true;
		}
		else{
			//retorna falso si no detencta nada
			return false;
		}
	}
	else{
		return false;
	}
}

bool SensorLimite::sc_2Verify(){
	// variables de los colores detectados
    uint16_t r, g, b, c;

	if (estado2) {
      	// selecciona sc_2
      	scSel(3);
		vTaskDelay(pdMS_TO_TICKS(2));
      	// sc_1 lee el color
      	sc_2.getRawData(&r, &g, &b, &c);
      	// sc_2 determina si el color detectado es el mismo del limite
      	long difCol2 = abs(r - lcr) + abs(g - lcg) + abs(b - lcb);
      	if (difCol2 > limCol) {
      		//retorna verdadero al detectar el limite
			return true;
      	}
		else{
			//retorna falso si no detecta nada
			return false;
		}
    }
	else{
		return false;
	}
}
