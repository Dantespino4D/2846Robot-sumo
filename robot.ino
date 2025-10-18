#include <Adafruit_TCS34725.h>
#include <Arduino.h>
#include <Musica.h>
#include <NewPing.h>
#include <Wire.h>

// direccion del multiplexor
#define TCAADDR 0x70

// variables de la configuracion de los pines pwm
int freq = 5000;
int solut = 8;
int pwmC_1 = 0;
int pwmC_2 = 1;

// variables que establecen el tiemṕo
int tiempo1 = 2000; // tiempo que sigue avanzando despues de dejar de detectar
                    // al rival
int tiempo2 = 2000; // tiempo que retrocede al detectar el borde

// variables que cuentan el tiempo
unsigned long temp1 = 0;
unsigned long temp2 = 0;
unsigned long temp3 = 0;
unsigned long temp4 = 0;

// variables que definen limites
int maxd = 40;    // limite de los sensores ultrasonicos
int limCol = 200; // tolerancia del color

// alerta del limite
SemaphoreHandle_t alerta;
SemaphoreHandle_t alerta2;

// alerta de deteccion del rival
SemaphoreHandle_t enemigo;
SemaphoreHandle_t enemigo2;

// orden de una accion
QueueHandle_t orden;

// variables de control
bool estado = false;
bool estado2 = false;
bool start = false;
int modo = 0;
bool memo1 = false;
bool memo2 = false;
bool memo3 = false;
bool memo4 = false;

// variables de los pines(y la de distancia maxima)
int ini = 2;
int trig_1 = 13;
int echo_1 = 34;
int trig_2 = 12;
int echo_2 = 35;
int led_1 = 19;
int led_2 = 5;
int pwm_1 = 4;
int pwm_2 = 18;
int cal = 15;
int ledp_1 = 16;
int ledp_2 = 17;

// variables del color predeterminado
int redC = 800;
int green = 700;
int blue = 500;

// se crean los objetos sc_1 y sc_2
Adafruit_TCS34725 sc_1 =
    Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 sc_2 =
    Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

// arreglo de los pines de los puentes h
int mot[2][2] = {{26, 25}, {14, 27}};

// se crean objetos ojos_1 y ojos_2
NewPing ojos_1(trig_1, echo_1, maxd);
NewPing ojos_2(trig_2, echo_2, maxd);

// valores establecidos del limite(pasar mas tarde a variables)
uint16_t lcr = redC, lcg = green, lcb = blue;

// struct con los valores de cada movimiento de los motores
struct MotorAccion {
  int vel_1;
  int vel_2;
  int e[2][2];
};

// se crea la variable struct de la direccion a, b y giro
MotorAccion dir_a = {255, 255, {{1, 0}, {1, 0}}};

MotorAccion dir_b = {255, 255, {{0, 1}, {0, 1}}};

MotorAccion giro = {255, 255, {{0, 1}, {1, 0}}};

// funcion para probar el funcionamiento de cosas
void prueba(int a) {
  if (a == 0) {
    digitalWrite(ledp_1, HIGH);
    delay(1000);
  } else {
    digitalWrite(ledp_2, HIGH);
    delay(1000);
  }
}

// para el robot
void alto() {
  digitalWrite(mot[0][0], LOW);
  digitalWrite(mot[0][1], LOW);
  digitalWrite(mot[1][0], LOW);
  digitalWrite(mot[1][1], LOW);
}
// funcion que selecciona que sensor de color usar(no me pregunten como
// funciona)
void scSel(uint8_t i) {
  if (i > 7)
    return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}
// logica de la calibracion
void calCol() {
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

// TAREA DE LA LOGICA DEL ROBOT

void robot(void *pvParameters) {
  // prende al precionar el boton
  while (digitalRead(ini) == LOW) {
    start = true;
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  while (1) {
    // inicia

    // condiciones que evaluan si ya pasaron los tiempos
    if (millis() - temp1 >= tiempo1) {
      memo1 = false;
    }
    if (millis() - temp2 >= tiempo1) {
      memo2 = false;
    }
    if (millis() - temp3 >= tiempo2) {
      memo3 = false;
    }
    if (millis() - temp4 >= tiempo2) {
      memo4 = false;
    }

    // MAQUINA DE ESTADOS

    // selecciona el estado

    // si detecta el limite por sc_1
    if (xSemaphoreTake(alerta, 0) == pdTRUE || memo3) {
      modo = 0;
    }
    // si detecta el limite por sc_2
    else if (xSemaphoreTake(alerta2, 0) == pdTRUE || memo4) {
      modo = 1;
    }
    // si deja de detectar al robot por ojos 1
    else if (xSemaphoreTake(enemigo, 0)) {
      modo = 2;
    }
    // si deja de detectar al robot por ojos 2
    else if (xSemaphoreTake(enemigo2, 0)) {
      modo = 3;
    }
    // si detecta el robot por ojos 1
    else if (memo1) {
      modo = 4;
    }
    // si detecta el robot por ojos 2
    else if (memo2) {
      modo = 5;
    }
    // si no detecta nada
    else {
      modo = 6;
    }

    // ejecuta el estado
    switch (modo) {
    // detiene el movimiento y retrocede en direccion b
    case 0:
      xQueueSend(orden, &dir_b, 0);
      memo3 = true;
      temp4 = millis();
      break;
      // detiene el movimiento y retrocede en direccion a
    case 1:
      xQueueSend(orden, &dir_a, 0);
      memo4 = true;
      temp3 = millis();
      break;
    // avanza por un tiempo definido de 4 segundo en direccion a
    case 2:
      xQueueSend(orden, &dir_a, 0);
      memo1 = true;
      temp1 = millis();
      temp3 = millis();
      temp4 = millis();
      break;
    // avanza por un tiempo definido de 4 segundos en direccion b
    case 3:
      xQueueSend(orden, &dir_b, 0);
      memo2 = true;
      temp2 = millis();
      temp3 = millis();
      temp4 = millis();
      break;
    // avanza en direccion a
    case 4:
      xQueueSend(orden, &dir_a, 0);
      temp3 = millis();
      temp4 = millis();
      break;
    // avanza en direccion b
    case 5:
      xQueueSend(orden, &dir_b, 0);
      temp3 = millis();
      temp4 = millis();
      break;
      // da vueltas hasta encontrar el robot
    case 6:
      xQueueSend(orden, &giro, 0);
      temp3 = millis();
      temp4 = millis();
      break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// TAREA DE LOS MOTORES

void motores(void *pvPrarmeters) {
  for (;;) {
    // se crea la variable donde se guardara la acion;
    MotorAccion accion;

    // se recive la accion
    xQueueReceive(orden, &accion, portMAX_DELAY);

    // se para todo, aplica velocidades y se encienden los que motores que se
    // necesitan
    alto();
    ledcWrite(pwmC_1, accion.vel_1);
    ledcWrite(pwmC_2, accion.vel_2);
    digitalWrite(mot[0][0], accion.e[0][0]);
    digitalWrite(mot[0][1], accion.e[0][1]);
    digitalWrite(mot[1][0], accion.e[1][0]);
    digitalWrite(mot[1][1], accion.e[1][1]);
  }
}

// TAREA DE LOS SENSORES DE LOS SENSORES DE COLOR

void senColor(void *pvParameters) {
  while (1) {
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
        // manda alerta para alejarse del limite
        xSemaphoreGive(alerta);
      }
    }

    // detecta si sc_2 funciona
    if (estado2) {
      // selecciona sc_2
      scSel(3);
      // sc_1 lee el color
      sc_2.getRawData(&r, &g, &b, &c);
      // sc_2 determina si el color detectado es el mismo del limite
      long difCol2 = abs(r - lcr) + abs(g - lcg) + abs(b - lcb);
      if (difCol2 > limCol) {
        // manda alerta para alejarse del limite
        xSemaphoreGive(alerta2);
      }
    }

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

// TAREA DE LOS SENSORES ultrasonicos

void senUltra(void *pvParameters) {
  while (4) {
    int dist_1 = ojos_1.ping_cm();
    int dist_2 = ojos_2.ping_cm();
    if (dist_1 != 0) {
      xSemaphoreGive(enemigo);
    }
    if (dist_2 != 0) {
      xSemaphoreGive(enemigo2);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void musica(void *pvParameters) {
  while (2) {
    while (start) {
      adestes();
      vTaskDelay(10);
    }
    vTaskDelay(10);
  }
}

// setup
void setup() {
  // se crea la alerta de deteccion del limite
  alerta = xSemaphoreCreateBinary();
  alerta2 = xSemaphoreCreateBinary();

  // se crean las alertas de deteccion de enemigos
  enemigo = xSemaphoreCreateBinary();
  enemigo2 = xSemaphoreCreateBinary();

  // se crea la la orden con la que se estara comunicando con los motores
  orden = xQueueCreate(5, sizeof(MotorAccion));

  // configuaracion de pines de los sensores de color
  Wire.begin();

  // se inicializan los pines
  pinMode(echo_1, INPUT);
  pinMode(echo_2, INPUT);
  pinMode(trig_1, OUTPUT);
  pinMode(trig_2, OUTPUT);
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(ini, INPUT_PULLUP);
  pinMode(cal, INPUT_PULLUP);
  pinMode(23, OUTPUT);
  pinMode(ledp_1, OUTPUT);
  pinMode(ledp_2, OUTPUT);

  //  se inicializan los pines de los motores(puentes h)
  pinMode(mot[0][0], OUTPUT);
  pinMode(mot[0][1], OUTPUT);
  pinMode(mot[1][0], OUTPUT);
  pinMode(mot[1][1], OUTPUT);

  // configuracion y asignacion de los pines pwm
  ledcSetup(pwmC_1, freq, solut);
  ledcSetup(pwmC_2, freq, solut);
  ledcAttachPin(pwm_1, pwmC_1);
  ledcAttachPin(pwm_2, pwmC_2);

  // selecciona sc_1
  scSel(0);
  // verifica el funcionamiento de sc_1
  if (sc_1.begin()) {
    // todo bien
    estado = true;
    prueba(0);
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
    prueba(1);
  } else {
    // no funciona y desantiva su funcionamiento
    estado2 = false;
  }
  calCol();

  // se crean las tareas
  xTaskCreatePinnedToCore(robot, "robot", 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(motores, "motores", 1024, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(senColor, "sensorColor", 2048, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(senUltra, "SensorUltra", 2048, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(musica, "musica", 1024, NULL, 1, NULL, 0);
}
void loop() {
  // DESCRIPCIONES A TOMAR EN CUENTA:
  // ojos_1 y sc_1 en direccion "a"
  // ojos_2 y sc_2 en direccion "b"
  // eliminar todas la funciones de prueba una vez armado y soldado el circuito
  // y se verifique que este todo correcto
}
