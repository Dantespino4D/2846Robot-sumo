#ifndef Musica_h
#define Musica_h

#include "driver/gpio.h"

const int NEG = 500;
const float BPUNT = NEG * 3;
const float RED = NEG * 4;
const float CPPUNT = NEG * 7/8;
const float BLA = NEG * 2;
const float PUNT = NEG * 3/2;
const float CPUNT = 375 * 3/4;
const float COR = NEG / 2;
const float SEMI = NEG / 4;
const float SPUNT = NEG / 8;
const int ESPA = 20;

// notas
const float C1 = 32.7;
const float DB1 = 34.65;
const float D1 = 36.71;
const float EB1 = 38.89;
const float E1 = 41.2;
const float F1 = 43.65;
const float GB1 = 44.25;
const float G1 = 49;
const float AB1 = 51.91;
const float L1 = 55;
const float BB1 = 58.27;
const float S1 = 61.74;
const float C2 = 65.41;
const float DB2 = 69.3;
const float D2 = 73.42;
const float EB2 = 77.78;
const float E2 = 82.41;
const float F2 = 87.31;
const float GB2 = 92.5;
const float G2 = 98;
const float AB2 = 103.83;
const float L2 = 110;
const float BB2 = 116.54;
const float B2 = 123.47;
const float C3 = 130.81;
const float DB3 = 138.59;
const float D3 = 146.83;
const float EB3 = 155.56;
const float E3 = 164.81;
const float F3 = 174.61;
const float GB3 = 185;
const float G3 = 196;
const float AB3 = 207.65;
const float L3 = 221.00;
const float BB3 = 233.08;
const float B3 = 246.94;
const float C4 = 261.63;
const float DB4 = 277.18;
const float D4 = 293.66;
const float EB4 = 311.13;
const float E4 = 329.63;
const float F4 = 349.23;
const float GB4 = 369.99;
const float G4 = 392.00;
const float AB4 = 415.30;
const float L4 = 441.00;
const float BB4 = 466.16;
const float B4 = 493.88;
const float C5 = 523.25;
const float DB5 = 554.37;
const float D5 = 587.33;
const float EB5 = 622.25;
const float E5 = 659.25;
const float F5 = 698.46;
const float GB5 = 739.99;
const float G5 = 783.99;
const float AB5 = 830.61;
const float L5 = 881;
const float BB5 = 932.33;
const float B5 = 987.77;
const float Notas[] = {
  C1, DB1, D1, EB1, E1, F1, GB1, G1, AB1, L1, BB1, S1,
  C2, DB2, D2, EB2, E2, F2, GB2, G2, AB2, L2, BB2, B2,
  C3, DB3, D3, EB3, E3, F3, GB3, G3, AB3, L3, BB3, B3,
  C4, DB4, D4, EB4, E4, F4, GB4, G4, AB4, L4, BB4, B4,
  C5, DB5, D5, EB5, E5, F5, GB5, G5, AB5, L5, BB5, B5
};

void pinMus(gpio_num_t pin);
void nota(float no, float dur);
void sil(float dur);
void adestes();
void martinillo();
void auxilio();
void plantera();
void boss1();
void funky();
void prueba();

#endif
