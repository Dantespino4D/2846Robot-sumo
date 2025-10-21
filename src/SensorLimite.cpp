#include <Arduino.h>
#include "SensorLimite.h"
#include <Adafruit_TCS34725.h>
#include <Wire.h>

// direccion del multiplexor
#define TCAADDR 0x70

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

    //se inicializa los objetos de los sensores de color
    sc_1(Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X)),
    sc_2(Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X))
{}

void SensorLimite::scSel(uint8_t i){
	if(i > 7){
	return;
	}
	Wire.beginTransmission(TCAADDR);
	Wire.write(1 << i);
	Wire.endTransmission();
}

void SensorLimite::calCol(){
 	// Acumuladores para promediar las lecturas
  	uint32_t t_r = 0;
  	uint32_t t_g = 0;
  	uint32_t t_b = 0;

  	uint16_t r, g, b, c;
  	const int NUMM = 50;
  	for (int i = 0; i < NUMM; i++) {
  	  	// Leer sensor 1
  	  	scSel(0);
  	  	sc_1.getRawData(&r, &g, &b, &c);
  	  	t_r += r;
  	  	t_g += g;
  	  	t_b += b;

	    delay(20);
  	}
  	// calcula el promedio de las muestras
 	 lcr = t_r / NUMM;
  	lcg = t_g / NUMM;
  	lcb = t_b / NUMM;
}

void SensorLimite::begin(){
	Wire.begin();

	// selecciona sc_1
	scSel(0);
	// verifica el funcionamiento de sc_1
	if (sc_1.begin()) {
		// todo bien
	    estado = true;
	} else {
	    // no funciona y desantiva su funcionamiento
    	estado = false;
	}

	// selecciona sc_2
	scSel(3);
	// verifica el funcionamiento de sc_2
	if (sc_2.begin()) {
	    // todo bien
	    estado2 = true;
	} else {
		// no funciona y desantiva su funcionamiento
	    estado2 = false;
	}
	calCol();
}

bool SensorLimite::sc_1Verify(){
	// variables de los colores detectados
    uint16_t r, g, b, c;
    // detecta si el sensor de color funciona bien
    if (estado) {
    	// selecciona sc_1
      	scSel(0);
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
