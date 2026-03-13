#ifndef CONFIGURACION_H
#define CONFIGURACION_H

// variables que establecen el tiemṕo
extern int tiempo1; // tiempo que retrocede al detectar el borde
extern int tiempo2; // tiempo que sigue avanzando despues de dejar de detectar al oponente a corto plazo
extern int tiempo3; // tiempo que sigue avanzando despues de dejar de detectar al oponente a largo plazo
extern int tiempo4; // tiempo que avanza en linea recta para buscar al oponente
extern int tiempo5; // tiempo en el que gira para buscar al oponente

// variables que definen limites
extern int maxd;    // limite de los sensores ultrasonicos (mm)
extern int limCol; // tolerancia del sendor de color

#endif // CONFIGURACION_H
