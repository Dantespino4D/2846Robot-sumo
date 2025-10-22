#include "ControlMotores.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include <Arduino.h>
#include <Musica.h>

// comandos de movimiento
#define ALTO 0
#define DIR_A 1
#define DIR_B 2
#define GIRO 3

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
int limCol = 200; // tolerancia del sendor de color

// alerta del limite
SemaphoreHandle_t alerta;
SemaphoreHandle_t alerta2;

// alerta de deteccion del rival
SemaphoreHandle_t enemigo;
SemaphoreHandle_t enemigo2;

// orden de una accion
QueueHandle_t orden;

// variables de control
bool start = false;
int modo = 6;
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
int ledp_1 = 16;
int ledp_2 = 17;

// variables de los pines de los motores
int pwm_1 = 4;
int pwm_2 = 18;
int mot[2][2] = {{26, 25}, {14, 27}};

// objeto de los sensores de color
SensorLimite sc(limCol);

// objeto de los sensores ultrasonicos
SensorRival su(maxd, trig_1, echo_1, trig_2, echo_2);

// objeto del controlador de motores
ControlMotores cm(pwm_1, pwm_2, mot[0][0], mot[0][1], mot[1][0], mot[1][1]);

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

// TAREA DE LA LOGICA DEL ROBOT

void robot(void *pvParameters) {
  // prende al precionar el boton
  while (digitalRead(ini) == HIGH) {
    start = true;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  while (true) {
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

    // variable del comandos
    int com;

    // ejecuta el estado
    switch (modo) {
    // detiene el movimiento y retrocede en direccion b
    case 0:
      com = DIR_B;
      xQueueSend(orden, &com, 0);
      memo3 = true;
      temp4 = millis();
      break;
      // detiene el movimiento y retrocede en direccion a
    case 1:
      com = DIR_A;
      xQueueSend(orden, &com, 0);
      memo4 = true;
      temp3 = millis();
      break;
    // avanza por un tiempo definido de 4 segundo en direccion a
    case 2:
      com = DIR_A;
      xQueueSend(orden, &com, 0);
      memo1 = true;
      temp1 = millis();
      temp3 = millis();
      temp4 = millis();
      break;
    // avanza por un tiempo definido de 4 segundos en direccion b
    case 3:
      com = DIR_B;
      xQueueSend(orden, &com, 0);
      memo2 = true;
      temp2 = millis();
      temp3 = millis();
      temp4 = millis();
      break;
    // avanza en direccion a
    case 4:
      com = DIR_A;
      xQueueSend(orden, &com, 0);
      temp3 = millis();
      temp4 = millis();
      break;
    // avanza en direccion b
    case 5:
      com = DIR_B;
      xQueueSend(orden, &com, 0);
      temp3 = millis();
      temp4 = millis();
      break;
      // da vueltas hasta encontrar el robot
    case 6:
      com = GIRO;
      xQueueSend(orden, &com, 0);
      temp3 = millis();
      temp4 = millis();
      break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// TAREA DE LOS MOTORES

void motores(void *pvPrarmeters) {
  int accion = 0;
  int accionNueva;

  while (true) {
    // Espera una nueva orden indefinidamente
    if (xQueueReceive(orden, &accionNueva, portMAX_DELAY) == pdPASS) {
      // Actualiza solo cuando llega algo nuevo
      accion = accionNueva;

      // Aplica el nuevo movimiento
      cm.controlador(accion);
    }
  }
}

// TAREA DE LOS SENSORES DE LOS SENSORES DE COLOR

void senColor(void *pvParameters) {
  while (true) {
    if (sc.sc_1Verify()) {
      xSemaphoreGive(alerta);
    }

    if (sc.sc_2Verify()) {
      xSemaphoreGive(alerta2);
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

// TAREA DE LOS SENSORES ULTRASONICOS

void senUltra(void *pvParameters) {
  while (true) {
    if (su.ojos_1Verify()) {
      xSemaphoreGive(enemigo);
    }
    if (su.ojos_2Verify()) {
      xSemaphoreGive(enemigo2);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void musica(void *pvParameters) {
  while (true) {
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
  orden = xQueueCreate(5, sizeof(int));

  // se inicializan los pines
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(ini, INPUT_PULLUP);
  pinMode(23, OUTPUT);
  pinMode(ledp_1, OUTPUT);
  pinMode(ledp_2, OUTPUT);

  sc.begin();
  cm.begin();

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
  // ojos_1 enemigo
  // ojos_2 enemigo2
  // sc_1 alerta
  // sc_2 alerta2
  // eliminar todas la funciones de prueba una vez armado y soldado el circuito
  // y se verifique que este todo correcto
}
