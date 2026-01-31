#include "sdkconfig.h"
#include "Wifi.h"
#include "Mqtt.h"
#include "ControlMotores.h"
#include "MaquinaEstados.h"
#include "Multiplexor.h"
#include "SensorLimite.h"
#include "SensorRival.h"
#include "SensorTof.h"
#include "SensorUltra.h"
#include "Telemetria.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/projdefs.h"
#include "portmacro.h"
#include "rgb.h"
#include "Nvs.h"
#include <Musica.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include <cstddef>
#include <cstdint>
#include <string>

static const char* TAG = "main";


// variables que establecen el tiemṕo
int tiempo1 = 2000; // tiempo que sigue avanzando despues de dejar de detectar
                    // al rival
int tiempo2 = 400; // tiempo que retrocede al detectar el borde

int tiempo3 = 2000;//tiempo que avanza en linea recta para buscar al oponente

int tiempo4 = 500; //tiempo en el que gira para buscar al oponente
// variables que definen limites
int maxd = 400;    // limite de los sensores ultrasonicos (mm)
int limCol = 200; // tolerancia del sendor de color

//creamos el mutex
SemaphoreHandle_t mutex = NULL;

// variables de control
 volatile bool start = false;

// variables de los pines
#ifdef CONFIG_IDF_TARGET_ESP32S3
    gpio_num_t mus = GPIO_NUM_47;
    gpio_num_t ini = GPIO_NUM_2;

    // Ultrasonicos
    gpio_num_t trig_1 = GPIO_NUM_15;
    gpio_num_t echo_1 = GPIO_NUM_21;
    gpio_num_t trig_2 = GPIO_NUM_38;
    gpio_num_t echo_2 = GPIO_NUM_48;


    gpio_num_t mot[2][2] = {{GPIO_NUM_4, GPIO_NUM_5},{GPIO_NUM_6, GPIO_NUM_7}};
#else
    // Configuración original ESP32
    gpio_num_t mus = GPIO_NUM_4;
    gpio_num_t ini = GPIO_NUM_26;
    gpio_num_t trig_1 = GPIO_NUM_19;
    gpio_num_t echo_1 = GPIO_NUM_23;
    gpio_num_t trig_2 = GPIO_NUM_18;
    gpio_num_t echo_2 = GPIO_NUM_25;

    // variables de los pines de los motores
    gpio_num_t mot[2][2] = {{GPIO_NUM_14, GPIO_NUM_13},{GPIO_NUM_27, GPIO_NUM_12}};
#endif

//objeto de Wifi
Wifi wi;

//objeto del protocolo MQTT
Mqtt mq;

//objeto del Multiplexor
Multiplexor mu;

// objeto de los sensores de color
SensorLimite* sc = nullptr;

// objeto del sensor rival
SensorRival* sr = nullptr;

// objeto del controlador de motores
ControlMotores cm(mot[0][1], mot[1][1], mot[0][0], mot[1][0]);

// puntero de la maquina de estados
MaquinaEstados *me = nullptr;

// objeto de la telemetria
Telemetria* tm = nullptr;

//objeto de handle de la tarea del robot
TaskHandle_t rob = NULL;


//handle de la tarea de los motores
TaskHandle_t motr = NULL;


// TAREA DE LA LOGICA DEL ROBOT

void robot(void *pvParameters) {
  	// prende al precionar el boton
  	while (gpio_get_level(ini) == 1) {
    	vTaskDelay(pdMS_TO_TICKS(100));
  	}
  	ESP_LOGI(TAG, "boton precionado");
  	sc->calCol();
  	vTaskDelay(pdMS_TO_TICKS(5000));
	rgb(1023, 0);
	ESP_LOGI(TAG,"iniciando combate");
  	start = true;
	uint64_t Tpas = 0;

	//bucle del robot
	while (true) {
		// inicia

	  	uint64_t Tini = esp_timer_get_time();
    	// MAQUINA DE ESTADOS
    	me->logica();

		//se calcula y envia la duracion de un ciclo
		uint64_t Tfin = esp_timer_get_time();
		int ciclo = (int)((Tfin - Tini)/1000);
		me->cicloR(ciclo, 1);
  	}
}

// TAREA DE LOS MOTORES

void motores(void *pvParameters) {
  	uint32_t accion = 0;
  	uint32_t accionNueva;

  	while (true) {
    	// Espera una nueva orden indefinidamente
    	if (xTaskNotifyWait(0, 0, &accionNueva, portMAX_DELAY) == pdPASS) {
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
		if (start) {
			if (sc->sc_1Verify()) {
				xTaskNotify(rob, (1 << 0), eSetBits);
    		}

    		if (sc->sc_2Verify()) {
				xTaskNotify(rob, (1 << 1), eSetBits);
    		}
		}
   	 	vTaskDelay(pdMS_TO_TICKS(10));
  	}
}

// TAREA DE LOS SENSORES ULTRASONICOS

void senUltra(void *pvParameters) {
	while (true) {
		if(start){
    		sr->procesar(&rob);
		}
    	vTaskDelay(pdMS_TO_TICKS(20));
  	}
}

//TAREA DE LA MUSICA

void musica(void *pvParameters) {
  	while (true) {
    	while (start) {
    		adestes();
      		vTaskDelay(10);
    	}
    	vTaskDelay(10);
  	}
}

//TAREA DE LA TELEMETRIA

void telemetria(void *pvParameters){
	while(true){
		tm->enviar();
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

// setup
extern "C" void app_main(void){
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        ESP_LOGI(TAG, "TARGET DETECTADO: ESP32-S3");
    #else
        ESP_LOGW(TAG, "TARGET DETECTADO: ESP32 ESTANDAR (O MACRO NO DEFINIDA)");
    #endif

	//inicializar la memoria nvs
	esp_err_t err = nvs_flash_init();
	if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
      	err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);

	//se lee las instrucciones del monitor serial
	Nvs sys("sistema");
	gpio_reset_pin(ini);
    gpio_set_direction(ini, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ini, GPIO_PULLUP_ONLY);
	vTaskDelay(pdMS_TO_TICKS(100));

	int32_t modo;
	if(gpio_get_level(ini) == 0){
		ESP_LOGI(TAG, "modo de prueba activado por boton");
		sys.guardar("modo", 0);
		modo = 0;
	}else{
        modo = sys.leer("modo", 1);
	}
	int32_t val = sys.leer("monitor", 2);
	//se aplica el valor elegido
    switch(val){
		case 0:
			esp_log_level_set("*", ESP_LOG_NONE);
			break;
        case 1:
			esp_log_level_set("*", ESP_LOG_ERROR);
			break;
		case 2:
			esp_log_level_set("*", ESP_LOG_INFO);
			break;
		case 3:
			esp_log_level_set("*", ESP_LOG_VERBOSE);
			break;
		default:
			esp_log_level_set("*", ESP_LOG_INFO);
			break;
	}


  	//se inicializan los canales y pines del led rgb
  	pwm_rgb();
  	rgb(1023, 512);

	//inicializamos el mutex
	mutex = xSemaphoreCreateMutex();

  	//ajustes iniciales
    ESP_LOGI(TAG, "Iniciando hardware...");
  	mu.begin();
  	cm.begin();
  	cm.alto();

  	//pin de la musica
  	pinMus(mus);


  	// se inicializan los pines output
	gpio_config_t io_conf_output;
	io_conf_output.pin_bit_mask = (1ULL << trig_1) | (1ULL << trig_2);
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

	//se le asigna al puntero los el objeto correspondiente
	sc = new SensorLimite(limCol, &mu, &mutex);
    if (sc == nullptr) {
        ESP_LOGE(TAG, "CRITICAL: sc is NULL after new!");
    } else {
        ESP_LOGI(TAG, "sc allocated at: %p", sc);
        // configuracion de los objetos de sensores de color y ultrasonicos
        sc->begin();
    }

	//se inicializa el sensor rival
	sr = new SensorUltra(maxd, trig_1, echo_1, trig_2, echo_2);
	sr->begin();

	//se inicializa la maquina de estados
    me = new MaquinaEstados(tiempo1, tiempo2, tiempo3, tiempo4, &motr);
	ESP_LOGI(TAG, "se inicializo todo");

	if(modo == 0){
		//modo de prueba
        ESP_LOGI(TAG, "Modo 0: Test y Telemetria");

		//inicializar el tcp/ip
		esp_netif_init();
		esp_event_loop_create_default();

		//se inicializa el wifi y el MQTT
 		wi.begin();
		wi.espera();
 		mq.begin();

		//se inicializa el objeto de telemetria
		tm = new Telemetria(me, &cm, sc, sr, &mq, &wi); // sr pasado DIRECTAMENTE

		//tarea de telemetria
		xTaskCreatePinnedToCore(telemetria, "telemetria", 8192, NULL, 1, NULL, 0);
	}else{
		//esp_log_level_set("*", ESP_LOG_NONE);
	}

  	// se crean las tareas
  	xTaskCreatePinnedToCore(robot, "robot", 4096, NULL, 2, &rob, 1);
  	xTaskCreatePinnedToCore(motores, "motores", 2048, NULL, 5, &motr, 1);
  	xTaskCreatePinnedToCore(senColor, "sensorColor", 2048, NULL, 3, NULL, 1);
	xTaskCreatePinnedToCore(senUltra, "SensorUltra", 4096, NULL, 2, NULL, 0);
	xTaskCreatePinnedToCore(musica, "musica", 1024, NULL, 1, NULL, 0);
	ESP_LOGI(TAG, "se inicializo las tareas");
}
   // DESCRIPCIONES A TOMAR EN CUENTA:
  	// ojos_1 y sc_1 en direccion "a"
  	// ojos_2 y sc_2 en direccion "b"
  	// ojos_1 enemigo
  	// ojos_2 enemigo2
  	// sc_1 alerta
  	// sc_2 alerta2
  	// y se verifique que este todo correcto
