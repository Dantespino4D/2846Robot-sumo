#include "configuracion.h"
#include <cstdint>

// variables que establecen el tiemṕo
int tiempo1 = 400; // tiempo que retrocede al detectar el borde
int tiempo2 = 250; // tiempo que sigue avanzando despues de dejar de detectar al oponente a corto plazo
int tiempo3 = 2000; // tiempo que sigue avanzando despues de dejar de detectar al oponente a largo plazo
int tiempo4 = 2000; // tiempo que avanza en linea recta para buscar al oponente
int tiempo5 = 500; // tiempo en el que gira para buscar al oponente

// variables que definen limites
int maxd = 400;    // limite de los sensores (mm)
int limCol = 200; // tolerancia del sensor de color e infrarrojo
uint8_t can[6] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35}; // Direcciones ToF
