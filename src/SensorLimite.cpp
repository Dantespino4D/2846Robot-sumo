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

// direccion del multiplexor y sensor
#define TCAADDR 0x70
#define TCSADDR 0x29 // Dirección I2C estándar del TCS34725
#define TAG "SensorLimite"

#define TCS_ENABLE 0x80 | 0x00
#define TCS_ATIME 0x80 | 0x01
#define TCS_CONTROL 0x80 | 0x0F
#define TCS_CDATAL 0x80 | 0x14

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
    lcb(50)
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
}

bool SensorLimite::read(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c){
	uint8_t write_buf[1] = {TCS_CDATAL};
	uint8_t read_buf[8];

	//definir donde empezar a escribir los datos leidos
	esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf, 1, pdMS_TO_TICKS(1000));
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
  	  	scSel(0);
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
	//se configura la comunicacion i2c
	i2c();

	uint8_t write_buf[2];

    write_buf[0] = TCS_ATIME;
    write_buf[1] = 0xFF;
	uint8_t write_buf_gain[2] = {TCS_CONTROL, 0x01};
	uint8_t write_buf_enable[2] = {TCS_ENABLE, 0x03};

	// selecciona sc_1
	scSel(0);
	vTaskDelay(pdMS_TO_TICKS(2));
	// verifica el funcionamiento de sc_1
	if (i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
        estado = true;
    } else {
	    estado = false;
		ESP_LOGE(TAG, "No se pudo inicializar sc_1 (TCS34725)");
	}

	scSel(3);
	vTaskDelay(pdMS_TO_TICKS(2));

    // Intentamos escribir la configuración
    if (i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_gain, 2, pdMS_TO_TICKS(100)) == ESP_OK &&
        i2c_master_write_to_device(I2C_MASTER_NUM, TCSADDR, write_buf_enable, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
	    estado2 = true;
	} else {
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
      	if(read(&r, &g, &b, &c)){
      		// sc_1 determina si el color detectado es el mismo del limite
      		long difCol = labs(r - lcr) + labs(g - lcg) + labs(b - lcb);
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
      	// sc_2 lee el color
      	if(read(&r, &g, &b, &c)){
      		// sc_2 determina si el color detectado es el mismo del limite
      		long difCol = labs(r - lcr) + labs(g - lcg) + labs(b - lcb);
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
	else{
		return false;
	}
}
