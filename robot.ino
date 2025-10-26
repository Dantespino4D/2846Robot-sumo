#include "src/ControlMotores.h"
#include "src/MaquinaEstados.h"
#include "src/SensorLimite.h"
#include "src/SensorRival.h"
#include <Arduino.h>
#include <Musica.h>

// variables que establecen el tiemṕo
int tiempo1 = 2000; // tiempo que sigue avanzando despues de dejar de detectar
                    // al rival
int tiempo2 = 2000; // tiempo que retrocede al detectar el borde

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

// puntero de la maquina de estados
MaquinaEstados *me = nullptr;

// funcion para probar el funcionamiento de cosas
void prueba(int a) {
  if (a == 0) {
    digitalWrite(ledp_1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
  } else {
    digitalWrite(ledp_2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// TAREA DE LA LOGICA DEL ROBOT

void robot(void *pvParameters) {
  // prende al precionar el boton
  while (digitalRead(ini) == HIGH) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  start = true;
  vTaskDelay(pdMS_TO_TICKS(5000));
  while (true) {
    // inicia

    // MAQUINA DE ESTADOS
    me->logica();

    vTaskDelay(pdMS_TO_TICKS(10));
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
    vTaskDelay(pdMS_TO_TICKS(5));
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
    vTaskDelay(pdMS_TO_TICKS(50));
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

  // metodo de seguridad
  if (alerta == NULL || alerta2 == NULL || enemigo == NULL ||
      enemigo2 == NULL) {
    cm.alto(); // detener motores por seguridad
    delay(2000);
    ESP.restart();
  }

  // se inicializan los pines
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(ini, INPUT_PULLUP);
  pinMode(23, OUTPUT);
  pinMode(ledp_1, OUTPUT);
  pinMode(ledp_2, OUTPUT);

  // configuracion de los objetos de sensores de color y ultrasonicos
  sc.begin();
  cm.begin();

  // se inicializa el objeto de la maquina de estados
  static MaquinaEstados maquina(tiempo1, tiempo2, alerta, alerta2, enemigo,
                                enemigo2, orden);

  // se apunta al puntero
  me = &maquina;

  // se crean las tareas
  xTaskCreatePinnedToCore(robot, "robot", 2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(motores, "motores", 2048, NULL, 1, NULL, 1);
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
