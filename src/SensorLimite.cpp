#include "SensorLimite.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "rgb.h"

#define TCSADDR 0x29 // Dirección I2C estándar del TCS34725
#define TAG "SensorLimite"

#define TCS_ENABLE 0x80 | 0x00
#define TCS_ATIME 0x80 | 0x01
#define TCS_CONTROL 0x80 | 0x0F
#define TCS_CDATAL 0x80 | 0x14

//constructor
SensorLimite::SensorLimite(int _limCol, Multiplexor* _mu):
	//tolerancia de color
    limCol(_limCol),
	mu(_mu),

    //variables de la logica
    estado(false),
    estado2(false),

	//variables predeterminadas
    redC(50),
    green(50),
    blue(50),
    lcr(50),
	lcg(50),
    lcb(50)
{}

bool SensorLimite::read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c){
	uint8_t write_buf[1] = {TCS_CDATAL};
	uint8_t read_buf[8];

	//definir donde empezar a escribir los datos leidos
	esp_err_t err = i2c_master_write_to_device(mu->port(), TCSADDR, write_buf, 1, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        return false;
    }

	//leer datos del color
	err = i2c_master_read_from_device(I2C_MASTER_NUM, TCSADDR, read_buf, 8, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        return false;
    }

	//conbina los bytes para transformarlos a valores en 16bits
	*c = (read_buf[1] << 8) | read_buf[0];
    *r = (read_buf[3] << 8) | read_buf[2];
    *g = (read_buf[5] << 8) | read_buf[4];
    *b = (read_buf[7] << 8) | read_buf[6];

	return true;
}

void SensorLimite::calCol(){
 	// Acumuladores para promediar las lecturas
  	uint32_t t_r = 0;
  	uint32_t t_g = 0;
  	uint32_t t_b = 0;

  	uint16_t r, g, b, c;
  	const int NUMM = 15;
  	for (int i = 0; i < NUMM; i++) {
  	  	// Leer sensor 1
  	  	mu->scSel(0);
		vTaskDelay(pdMS_TO_TICKS(10));
  	  	if(read(&r, &g, &b, &c)){
			t_r += r;
  	  		t_g += g;
  	  		t_b += b;
		}
	    vTaskDelay(pdMS_TO_TICKS(10));
  	}
  	// calcula el promedio de las muestras
 	lcr = t_r / NUMM;
	lcg = t_g / NUMM;
  	lcb = t_b / NUMM;

	ESP_LOGI(TAG, "Calibracion: R=%d, G=%d, B=%d", lcr, lcg, lcb);
}

void SensorLimite::begin(){
	uint8_t write_buf[2];

    write_buf[0] = TCS_ATIME;
    write_buf[1] = 0xFF;
	uint8_t write_buf_gain[2] = {TCS_CONTROL, 0x01};
	uint8_t write_buf_enable[2] = {TCS_ENABLE, 0x03};

	// selecciona sc_1
	mu->scSel(0);
	vTaskDelay(pdMS_TO_TICKS(2));
	// verifica el funcionamiento de sc_1
	if (i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
        estado = true;
    } else {
	    estado = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_1 (TCS34725)");
		rgb(0, 1023);
	}

	mu->scSel(3);

	vTaskDelay(pdMS_TO_TICKS(2));

    // Intentamos escribir la configuración
    if (i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
	    estado2 = true;
	} else {
	    estado2 = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_2 (TCS34725)");
		rgb(0, 1023);
	}
}

bool SensorLimite::sc_1Verify(){
	// variables de los colores detectados
    uint16_t r, g, b, c;
    // detecta si el sensor de color funciona bien
    if (estado) {
    	// selecciona sc_1
      	mu->scSel(0);
		vTaskDelay(pdMS_TO_TICKS(2));
      	// sc_1 lee el color
      	if(read(&r, &g, &b, &c)){
      		// sc_1 determina si el color detectado es el mismo del limite
      		long difCol = labs(r - lcr) + labs(g - lcg) + labs(b - lcb);
      		if (difCol > limCol) {
        		// retorna verdadero al detectar el limite
				return true;
			}else{
					//retorna falso si no detencta nada
			return false;
			}
		}else{
			return false;
		}
	}else{
		return false;
		rgb(0, 1023);
	}
}

bool SensorLimite::sc_2Verify(){
	// variables de los colores detectados
    uint16_t r, g, b, c;

	if (estado2) {
      	// selecciona sc_2
      	mu->scSel(3);
		vTaskDelay(pdMS_TO_TICKS(2));
      	// sc_2 lee el color
      	if(read(&r, &g, &b, &c)){
      		// sc_2 determina si el color detectado es el mismo del limite
      		long difCol = labs(r - lcr) + labs(g - lcg) + labs(b - lcb);
      		if (difCol > limCol) {
        		// retorna verdadero al detectar el limite
				return true;
			}else{
					//retorna falso si no detencta nada
			return false;
			}
		}else{
			return false;
		}
    }
	else{
		return false;
		rgb(0, 1023);
	}
}
