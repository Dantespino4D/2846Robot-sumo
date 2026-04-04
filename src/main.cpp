#include "sdkconfig.h"
#include "comunicaciones/Wifi.h"
#include "comunicaciones/Mqtt.h"
#include "actuadores/ControlMotores.h"
#include "core/MaquinaEstados.h"
#include "actuadores/Multiplexor.h"
#include "sensores/SensorTcs.h"
#include "sensores/SensorRival.h"
#include "sensores/SensorTof.h"
#include "sensores/SensorUltra.h"
#include "comunicaciones/Telemetria.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "actuadores/rgb.h"
#include "core/Nvs.h"
#include "../lib/Musica/Musica.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include <cstdint>
#include "configuracion/configuracion.h"
#include "configuracion/pines.h"
#include "configuracion/eventos.h"

static const char* TAG = "main";

//event group para sincronizar tareas
EventGroupHandle_t eventos = NULL;

//creamos el mutex
SemaphoreHandle_t mutex = NULL;

// variables de control
 volatile bool start = false;
 int32_t modo;

//objeto de Wifi
Wifi wi;

//objeto del protocolo MQTT
Mqtt mq;

//objeto del Multiplexor
Multiplexor mu;

// objeto de los sensores de color
SensorTcs* sc = nullptr;

// objeto del sensor rival
SensorRival* sr = nullptr;

// objeto del controlador de motores
ControlMotores cm(MOT_A2, MOT_B2, MOT_A1, MOT_B1);

// puntero de la maquina de estados
MaquinaEstados *me = nullptr;

// objeto de la telemetria
Telemetria* tm = nullptr;

//objeto de handle de la tarea del robot
TaskHandle_t rob = NULL;

//handle de la tarea de los motores
TaskHandle_t motr = NULL;

//protoripos de las tareas
void robot(void *pvParameters);
void motores(void *pvParameters);
void senColor(void *pvParameters);
void senRival(void *pvParameters);
void musica(void *pvParameters);
void telemetria(void *pvParameters);
//prototipos de las funciones
void begin();
void begin_hardware();
void comunicaciones();

// APP MAIN

extern "C" void app_main(void){
	begin();

    begin_hardware();

    comunicaciones();

  	// se crean las tareas
  	xTaskCreatePinnedToCore(robot, "robot", 4096, NULL, 2, &rob, 1);
  	xTaskCreatePinnedToCore(motores, "motores", 2048, NULL, 5, &motr, 1);
  	xTaskCreatePinnedToCore(senColor, "sensorColor", 2048, NULL, 3, NULL, 1);
	xTaskCreatePinnedToCore(senRival, "SensorRival", 4096, NULL, 2, NULL, 0);
	xTaskCreatePinnedToCore(musica, "musica", 1024, NULL, 1, NULL, 0);
	ESP_LOGI(TAG, "se inicializo las tareas");
}


// LOGICA DEL ROBOT


void robot(void *pvParameters) {
  	// prende al precionar el boton
  	while (gpio_get_level(INI) == 1) {
    	vTaskDelay(pdMS_TO_TICKS(100));
  	}
  	ESP_LOGI(TAG, "boton precionado");
  	sc->calCol();
  	vTaskDelay(pdMS_TO_TICKS(5000));
	rgb(1023, 0);
	ESP_LOGI(TAG,"iniciando combate");
  	start = true;

	//bucle del robot
	while (true) {
		// inicia

		//verifica si hay algun error en el i2c
		if(!mu.verify()){
			//se detecta error, parar por seguridad
			rgb(0, 1023);
			cm.alto();
			if(xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE){
				//porceso para reiniciar el i2c
				mu.reinicio();
				mu.begin();
				sc->begin();
				sr->begin();
				rgb(1023, 0);
				xSemaphoreGive(mutex);
			}
		}
	  	uint64_t Tini = esp_timer_get_time();
    	// MAQUINA DE ESTADOS
    	me->logica();

		//se calcula y envia la duracion de un ciclo
		uint64_t Tfin = esp_timer_get_time();
		int ciclo = (int)((Tfin - Tini)/1000);
		me->cicloR(ciclo, 1);
		vTaskDelay(pdMS_TO_TICKS(10));
  	}
}


//MOTORES


void motores(void *pvParameters) {
  	uint32_t accion = 0;
  	uint32_t accionNueva;

  	while (true) {
    	// Espera una nueva orden indefinidamente
    	if (xTaskNotifyWait(0, 0, &accionNueva, portMAX_DELAY) == pdPASS) {
      		// Actualiza solo cuando llega algo nuevo
      		accion = accionNueva;
		}
      	// Aplica el nuevo movimiento
      	cm.controlador(accion);
	}
}


// SENSORES DE COLOR


void senColor(void *pvParameters) {
	while (true) {
		//inicia el combate
		if (start) {
			//se verifican ambos sensores de color
			sc->procesar();
		}
   	 	vTaskDelay(pdMS_TO_TICKS(10));
  	}
}


// SENSORES RIVAL


void senRival(void *pvParameters) {
	while (true) {
		//inicia el combate
		if(start){
			//se verifican los sensores de la deteccion del rival(tof o ultrasonicos)
    		sr->procesar();
		}
    	vTaskDelay(pdMS_TO_TICKS(20));
  	}
}


// MUSICA


void musica(void *pvParameters) {
  	while (true) {
    	while (start) {
			//se toca la musica
    		adestes();
      		vTaskDelay(10);
    	}
    	vTaskDelay(10);
  	}
}


// TELEMETRIA


void telemetria(void *pvParameters){
	while(true){
		//se manda la informacion actual para su procesamiento y analisis
		tm->enviar();
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}



// funcion que inicializa el sistema
void begin() {
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        ESP_LOGI(TAG, "TARGET DETECTADO: ESP32-S3");
    #else
        ESP_LOGW(TAG, "DETECTADO: ESP32 ESTANDAR");
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

	//se crea el event group
	eventos = xEventGroupCreate();
	if(eventos == NULL){
		ESP_LOGE(TAG, "Error al crear el event group");
	}

	//se inicializan pines input pullup
	gpio_config_t io_conf_input;
	io_conf_input.pin_bit_mask = (1ULL << INI);
	io_conf_input.mode = GPIO_MODE_INPUT;
	io_conf_input.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf_input.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf_input.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf_input);

	vTaskDelay(pdMS_TO_TICKS(100));

	if(gpio_get_level(INI) == 0){
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
			ESP_LOGI(TAG, "desactivando monitor");
			esp_log_level_set("*", ESP_LOG_NONE);
			break;
        case 1:
			ESP_LOGI(TAG, "monitor solo errores");
			esp_log_level_set("*", ESP_LOG_ERROR);
			break;
		case 2:
			ESP_LOGI(TAG, "monitor solo info");
			esp_log_level_set("*", ESP_LOG_INFO);
			break;
		case 3:
			ESP_LOGI(TAG, "monitor todo");
			esp_log_level_set("*", ESP_LOG_VERBOSE);
			break;
		default:
			ESP_LOGI(TAG, "monitor solo info");
			esp_log_level_set("*", ESP_LOG_INFO);
			break;
	}


  	//se inicializan los canales y pines del led rgb
  	pwm_rgb();
  	rgb(1023, 800);

	//inicializamos el mutex
	mutex = xSemaphoreCreateMutex();
}

//inicializacion del hardware
void begin_hardware() {
  	//ajustes iniciales
    ESP_LOGI(TAG, "Iniciando hardware...");
  	mu.begin();
  	cm.begin();
  	cm.alto();

  	//pin de la musica
  	pinMus(MUS);

	//se le asigna al puntero los el objeto correspondiente
	sc = new SensorTcs(limCol, &mu, &mutex);
    if (sc == nullptr) {
        ESP_LOGE(TAG, "CRITICAL: sc is NULL after new!");
    } else {
        ESP_LOGI(TAG, "sc allocated at: %p", sc);
        // configuracion de los objetos de sensores de color y ultrasonicos
        sc->begin();
    }

	//se inicializa el sensor rival
	sr = new SensorUltra(maxd);
	sr->begin();

	//se inicializa la maquina de estados
    me = new MaquinaEstados(tiempo1, tiempo2, tiempo3, tiempo4, tiempo5, &motr);
	ESP_LOGI(TAG, "se inicializo todo");
}

void comunicaciones() {
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
		ESP_LOGI(TAG, "monitor desactivado por modo combate");
		//esp_log_level_set("*", ESP_LOG_NONE);
	}
}
