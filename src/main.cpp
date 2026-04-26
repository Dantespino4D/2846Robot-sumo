#include "sdkconfig.h"
#include "esp_task_wdt.h"
#include "comunicaciones/Wifi.h"
#include "comunicaciones/Mqtt.h"
#include "actuadores/ControlMotores.h"
#include "core/MaquinaEstados.h"
#include "core/GestorI2C.h"
#include "sensores/SensorLimite.h"
#ifdef CONFIG_IDF_TARGET_ESP32S3
    #include "sensores/SensorTcrt.h"
    #include "sensores/SensorTof.h"
#else
    #include "sensores/SensorTcs.h"
    #include "sensores/SensorUltra.h"
#endif
#include "sensores/SensorRival.h"
#include "comunicaciones/Telemetria.h"
#include "core/MonitorSistema.h"
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

// variables de control
volatile bool start = false;
int32_t modo;

//objeto de Wifi
Wifi wi;

//objeto del protocolo MQTT
Mqtt mq;

//objeto que recopila datos del sistema
MonitorSistema mon;

//objeto que gestiona el bus I2C
GestorI2C i2c;

// objeto de los sensores de limite
SensorLimite* sl = nullptr;

// objeto del sensor rival
SensorRival* sr = nullptr;

// objeto del controlador de motores
ControlMotores cm(MOT_A2, MOT_B2, MOT_A1, MOT_B1);

// puntero de la maquina de estados
MaquinaEstados *me = nullptr;

// variable que define la version del hardware
bool final = false;

// objeto de la telemetria
Telemetria* tm = nullptr;

//handle de la tarea de los motores
TaskHandle_t motr = NULL;

//buffer para el TCB
StaticTask_t tcbRobot;
StaticTask_t tcbMotores;
StaticTask_t tcbMusica;
StaticTask_t tcbTelemetria;

//stack de las tareas
StackType_t stackRobot[4096];
StackType_t stackMotores[2048];
StackType_t stackMusica[1024];
StackType_t stackTelemetria[10240];



//protoripos de las tareas
void robot(void *pvParameters);
void motores(void *pvParameters);
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
  	xTaskCreateStaticPinnedToCore(robot, "robot", 4096, NULL, 2, stackRobot, &tcbRobot, 1);
  	motr = xTaskCreateStaticPinnedToCore(motores, "motores", 2048, NULL, 5, stackMotores, &tcbMotores, 1);
	xTaskCreateStaticPinnedToCore(musica, "musica", 1024, NULL, 1, stackMusica, &tcbMusica, 0);

	ESP_LOGI(TAG, "se inicializo las tareas");
}


// LOGICA DEL ROBOT


void robot(void *pvParameters) {
	//suscribir la tarea al watchdog
	esp_err_t err = esp_task_wdt_add(NULL);
	if(err != ESP_OK){
		ESP_LOGE(TAG, "Error al suscribir al watchdog: %s", esp_err_to_name(err));
	}
	esp_reset_reason_t rason = esp_reset_reason();
	bool watchdog = (rason == ESP_RST_TASK_WDT || rason == ESP_RST_WDT);
	if(!watchdog){
  		// prende al precionar el boton
  		while (gpio_get_level(INI) == 1) {
    		vTaskDelay(pdMS_TO_TICKS(100));
			esp_task_wdt_reset();
  		}

		//espera de 5 segundos
  		ESP_LOGI(TAG, "boton precionado");
		for(int i = 0; i < 50; i++){
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_task_wdt_reset(); // <-- VITAL
        }
	}else{
		ESP_LOGW(TAG, "Reinicio por Watchdog detectado, omitiendo boton");
	}
	//prende el led en verde para indicar que todo esta bien
	rgb(1023, 0);
	ESP_LOGI(TAG,"iniciando combate");

	//la variable star la cual activa las tareas
  	start = true;

	//bucle del robot
	while (true) {
		// inicia

		// se resetea el watchdog
		esp_task_wdt_reset();

	  	uint64_t Tini = esp_timer_get_time();

		// se obtienen las distancias de los sensores rivales
		uint16_t distBuffer[24] = {0};
		if (sr != nullptr) {
			sr->getDistancias(distBuffer);
		}

    	// MAQUINA DE ESTADOS
		me->corrienteA = mon.corrienteStall();
    	me->logica();

		//se calcula y envia la duracion de un ciclo
		uint64_t Tfin = esp_timer_get_time();
		int ciclo = (int)((Tfin - Tini)/1000);
		me->cicloR(ciclo, 1);
		vTaskDelay(pdMS_TO_TICKS(1));
  	}
}


//MOTORES


void motores(void *pvParameters) {
	//suscribir la tarea al watchdog
	esp_err_t err = esp_task_wdt_add(NULL);
	if(err != ESP_OK){
		ESP_LOGE(TAG, "Error al suscribir al watchdog: %s", esp_err_to_name(err));
	}

  	uint32_t accion = 0;
  	uint32_t accionNueva;

  	while (true) {
		esp_task_wdt_reset();
    	// Espera una nueva orden indefinidamente
    	if (xTaskNotifyWait(0, 0, &accionNueva, pdMS_TO_TICKS(50)) == pdPASS) {
      		// Actualiza solo cuando llega algo nuevo
      		accion = accionNueva;
		}
      	// Aplica el nuevo movimiento
      	cm.controlador(accion);
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

	//inicializar la memoria nvs personalizada
	esp_err_t err = nvs_flash_init_partition("configuracion");
	if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase_partition("configuracion"));
      	err = nvs_flash_init_partition("configuracion");
	}
	ESP_ERROR_CHECK(err);

	//inicializar el monitor del sistema
	err = nvs_flash_init();
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


  	pwm_rgb();
	mon.begin();
  	rgb(1023, 800);
}

//inicializacion del hardware
void begin_hardware() {
  	//ajustes iniciales
    ESP_LOGI(TAG, "Iniciando hardware...");
  	i2c.begin();
  	cm.begin();
  	cm.alto();

  	//pin de la musica
  	pinMus(MUS);

	//se le asigna al puntero los el objeto correspondiente
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        final = true;
        sl = new SensorTcrt(TCRT_1, TCRT_2);
        sr = new SensorTof(i2c, dir, maxd);
    #else
        final = false;
        sl = new SensorTcs(limCol, &i2c);
        sr = new SensorUltra(maxd);
    #endif

    if (sl != nullptr) {
        sl->begin();
    } else {
        ESP_LOGE(TAG, "No se pudo crear el sensor de limite");
    }

    if (sr != nullptr) {
        sr->begin();
    } else {
        ESP_LOGE(TAG, "No se pudo crear el sensor rival");
    }

	//se inicializa la maquina de estados
    me = new MaquinaEstados(tiempo1, tiempo2, tiempo3, tiempo4, tiempo5, &motr, final);
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

		tm = new Telemetria(me, &cm, sl, sr, &mq, &wi, &mon, final);

		//tarea de telemetria
		xTaskCreateStaticPinnedToCore(telemetria, "telemetria", sizeof(stackTelemetria), NULL, 1, stackTelemetria, &tcbTelemetria, 0);
	}else{
		ESP_LOGI(TAG, "monitor desactivado por modo combate");
		esp_log_level_set("*", ESP_LOG_NONE);
	}
}

// funcion que libera la memoria de los objetos creados
void limpiar_memoria() {
    if (tm != nullptr) { delete tm; tm = nullptr; }
    if (me != nullptr) { delete me; me = nullptr; }
    if (sr != nullptr) { delete sr; sr = nullptr; }
    if (sl != nullptr) { delete sl; sl = nullptr; }
    ESP_LOGI(TAG, "Memoria de objetos liberada.");
}
