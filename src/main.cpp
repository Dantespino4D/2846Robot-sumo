#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ControlMotores.h"
#include "MaquinaEstados.h"
#include "SensorLimite.h"
#include "SensorRival.h"
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
gpio_num_t ini = GPIO_NUM_2;
gpio_num_t trig_1 = GPIO_NUM_13;
gpio_num_t echo_1 = GPIO_NUM_34;
gpio_num_t trig_2 = GPIO_NUM_12;
gpio_num_t echo_2 = GPIO_NUM_35;
gpio_num_t led_1 = GPIO_NUM_19;
gpio_num_t led_2 = GPIO_NUM_5;
gpio_num_t ledp_1 = GPIO_NUM_16;
gpio_num_t ledp_2 = GPIO_NUM_17;

// variables de los pines de los motores
gpio_num_t pwm_1 = GPIO_NUM_4;
gpio_num_t pwm_2 = GPIO_NUM_18;
gpio_num_t mot[2][2] = {{GPIO_NUM_26, GPIO_NUM_25}, {GPIO_NUM_14, GPIO_NUM_27}};

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
    gpio_set_level(ledp_1, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
  } else {
    gpio_set_level(ledp_2, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// TAREA DE LA LOGICA DEL ROBOT

void robot(void *pvParameters) {
  // prende al precionar el boton
  while (gpio_get_level(ini) == 1) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  start = true;
  sc.calCol();
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
extern "C" void app_main(void){
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
    prueba(0);
    esp_restart();
  } else {
    prueba(1);
  }

  // se inicializan los pines output
	gpio_config_t io_conf_output;
	io_conf_output.pin_bit_mask = (1ULL << led_1) | (1ULL << led_2) | (1ULL << ledp_1) | (1ULL << trig_1) | (1ULL << trig_2) | (1ULL << ledp_2) | (1ULL << mot[0][0]) | (1ULL << mot[0][1]) | (1ULL << mot[1][0]) | (1ULL << mot[1][1]);
	io_conf_output.mode = GPIO_MODE_OUTPUT;
	io_conf_output.pull_up_en = GPIO_PULLUP_DISABLE;
	io_conf_output.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf_output.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf_output);

	//se inicializan pines input pullup
	gpio_config_t io_conf_input;
	io_conf_input.pin_bit_mask = (1ULL << ini);
	io_conf_input.mode = GPIO_MODE_INPUT;
	io_conf_input.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf_input.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf_input.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf_input);

	//se inicializan los pines input
   gpio_config_t io_conf_input_simple;
    io_conf_input_simple.pin_bit_mask = (1ULL << echo_1) | (1ULL << echo_2);
    io_conf_input_simple.mode = GPIO_MODE_INPUT;
    io_conf_input_simple.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf_input_simple.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf_input_simple.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf_input_simple);

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
  // DESCRIPCIONES A TOMAR EN CUENTA:
  // ojos_1 y sc_1 en direccion "a"
  // ojos_2 y sc_2 en direccion "b"
  // ojos_1 enemigo
  // ojos_2 enemigo2
  // sc_1 alerta
  // sc_2 alerta2
  // y se verifique que este todo correcto

