#ifndef Musica_h
#define Musica_h


#include <stdint.h>

// ================= TIEMPOS (Corregidos con precisión) =================
constexpr int NEG_INT = 500; // Valor base

// Usamos .0 para asegurar división flotante correcta
constexpr float NEG    = 500.0;
constexpr float BLA    = NEG * 2.0;
constexpr float RED    = NEG * 4.0;
constexpr float PUNT   = NEG * 1.5;      // Equivalente a 3/2
constexpr float BPUNT  = NEG * 3.0;
constexpr float CPUNT  = 375.0 * 0.75;   // Equivalente a 3/4
constexpr float CPPUNT = NEG * (7.0/8.0);
constexpr float COR    = NEG / 2.0;
constexpr float SEMI   = NEG / 4.0;
constexpr float SPUNT  = NEG / 8.0;

constexpr int ESPA = 20; // Espacio de silencio entre notas

// ================= NOTAS (Frecuencias) =================
constexpr float C1 = 32.70;
constexpr float DB1 = 34.65;
constexpr float D1 = 36.71;
constexpr float EB1 = 38.89;
constexpr float E1 = 41.20;
constexpr float F1 = 43.65;
constexpr float GB1 = 44.25;
constexpr float G1 = 49.00;
constexpr float AB1 = 51.91;
constexpr float L1 = 55.00;
constexpr float BB1 = 58.27;
constexpr float S1 = 61.74; // Asumo que S1 es B1 en otra notación
constexpr float C2 = 65.41;
constexpr float DB2 = 69.30;
constexpr float D2 = 73.42;
constexpr float EB2 = 77.78;
constexpr float E2 = 82.41;
constexpr float F2 = 87.31;
constexpr float GB2 = 92.50;
constexpr float G2 = 98.00;
constexpr float AB2 = 103.83;
constexpr float L2 = 110.00;
constexpr float BB2 = 116.54;
constexpr float B2 = 123.47;
constexpr float C3 = 130.81;
constexpr float DB3 = 138.59;
constexpr float D3 = 146.83;
constexpr float EB3 = 155.56;
constexpr float E3 = 164.81;
constexpr float F3 = 174.61;
constexpr float GB3 = 185.00;
constexpr float G3 = 196.00;
constexpr float AB3 = 207.65;
constexpr float L3 = 221.00;
constexpr float BB3 = 233.08;
constexpr float B3 = 246.94;
constexpr float C4 = 261.63;
constexpr float DB4 = 277.18;
constexpr float D4 = 293.66;
constexpr float EB4 = 311.13;
constexpr float E4 = 329.63;
constexpr float F4 = 349.23;
constexpr float GB4 = 369.99;
constexpr float G4 = 392.00;
constexpr float AB4 = 415.30;
constexpr float L4 = 441.00;
constexpr float BB4 = 466.16;
constexpr float B4 = 493.88;
constexpr float C5 = 523.25;
constexpr float DB5 = 554.37;
constexpr float D5 = 587.33;
constexpr float EB5 = 622.25;
constexpr float E5 = 659.25;
constexpr float F5 = 698.46;
constexpr float GB5 = 739.99;
constexpr float G5 = 783.99;
constexpr float AB5 = 830.61;
constexpr float L5 = 881.00;
constexpr float BB5 = 932.33;
constexpr float B5 = 987.77;

// Arreglo de notas (static constexpr para eficiencia)
inline static constexpr float Notas[] = {
  C1, DB1, D1, EB1, E1, F1, GB1, G1, AB1, L1, BB1, S1,
  C2, DB2, D2, EB2, E2, F2, GB2, G2, AB2, L2, BB2, B2,
  C3, DB3, D3, EB3, E3, F3, GB3, G3, AB3, L3, BB3, B3,
  C4, DB4, D4, EB4, E4, F4, GB4, G4, AB4, L4, BB4, B4,
  C5, DB5, D5, EB5, E5, F5, GB5, G5, AB5, L5, BB5, B5
};

void pinMus(int pin);
void nota(float no, float dur);
void sil(float dur);
void adestes();
void martinillo();
void auxilio();
void plantera();
void boss1();
void funky();
void vals2();
void prueba();

#endif
