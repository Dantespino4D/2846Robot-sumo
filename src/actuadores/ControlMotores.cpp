#include "driver/gpio.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_prelude.h"
#include "ControlMotores.h"
#include "../core/Nvs.h"
#include "../configuracion/pines.h"
#include "hal/mcpwm_types.h"
#include <cstdint>
#include <cstdlib>

#ifdef CONFIG_IDF_TARGET_ESP32S3
//    #define MODO_PWM LEDC_LOW_SPEED_MODE
#else
    #define MODO_PWM LEDC_HIGH_SPEED_MODE
#endif

ControlMotores::ControlMotores(gpio_num_t motA2, gpio_num_t motB2, gpio_num_t motA1, gpio_num_t motB1):
// Valores de configuracion pwm
    freq(20000000),
    period(1023),
	//valor del tiempo de la rampa
	tRam(200),

	//velocidades por defecto
	vel_nI(700),
	vel_nD(700),
	vel_aI(950),
	vel_aD(800),
	vel_mI(1023),
	vel_mD(1023),
	vel_pI(950),
	vel_pD(600),
	vel_gI(950),
	vel_gD(-800),

	vel1(0),
	vel2(0),

	rampa(false),

	vel1_obj(0),
	vel2_obj(0)
{
	//pines pwm
	mot2[0] = motA2;
    mot2[1] = motB2;
	//pines de los motores
    mot[0]  = motA1;
    mot[1]  = motB1;
}

//estblecer velocidad
void ControlMotores::velocidad(int16_t vel_1, int16_t vel_2, bool ram){
	//variable de las velocidades actuales
	vel1_obj = vel_1;
	vel2_obj = vel_2;

	//variable de la rampa
	rampa = ram;
}

//tarea que aplica la rampa
void ControlMotores::tareaRampa(void* arg){
	//se obtiene el puntero a la instancia de ControlMotores
	ControlMotores* self = static_cast<ControlMotores*>(arg);
	while(true){
		//se calcula el paso de incremento o decremento de la velocidad en cada ciclo de la rampa
		int8_t paso = 10230 / self->tRam;
		//se verifica si se ha alcanzado el objetivo de velocidad del lado izquierdo
		if(self->rampa){
			if(abs(self->vel1 - self->vel1_obj) > paso){
				//si no se ha alcanzado, se incrementa o decrementa la velocidad actual hacia el objetivo
				if(self->vel1 < self->vel1_obj){
					self->vel1 += paso;
				}else{
					self->vel1 -= paso;
				}
			}else{
				//si se ha alcanzado el objetivo, se asegura que la velocidad actual sea
				self->vel1 = self->vel1_obj;
			}
		}else{
			//si se ha alcanzado el objetivo, se asegura que la velocidad actual sea
			self->vel1 = self->vel1_obj;
		}

		//se verifica si se ha alcanzado el objetivo de velocidad del lado derecho
		if(self->rampa){
			if(abs(self->vel2 - self->vel2_obj) > paso){
				//si no se ha alcanzado, se incrementa o decrementa la velocidad actual hacia el objetivo
				if(self->vel2 < self->vel2_obj){
					self->vel2 += paso;
				}else{
					self->vel2 -= paso;
				}
			}else{
				//si se ha alcanzado el objetivo, se asegura que la velocidad actual sea
				self->vel2 = self->vel2_obj;
			}
		}else{
			//si se ha alcanzado el objetivo, se asegura que la velocidad actual sea
			self->vel2 = self->vel2_obj;
		}

		//se aplican las velocidades

		//calcula y aplica la velocidad del lado izquierdo
		if(self->vel1 < 0){
			//si es en reversa
			mcpwm_generator_set_force_level(self->gen_izq_a, 0, true);
			mcpwm_generator_set_force_level(self->gen_izq_b, -1, true);
			mcpwm_comparator_set_compare_value(self->cmpr_izq, abs(self->vel1));
		}else if(self->vel1 > 0){
			//si es hacia adelante
			mcpwm_generator_set_force_level(self->gen_izq_a, -1, true);
			mcpwm_generator_set_force_level(self->gen_izq_b, 0, true);
			mcpwm_comparator_set_compare_value(self->cmpr_izq, self->vel1);
		}else{
			//si debe frenar
			mcpwm_generator_set_force_level(self->gen_izq_a, 1, true);
			mcpwm_generator_set_force_level(self->gen_izq_b, 1, true);
		}

		//calcula y aplica la velocidad del lado derecho
		if(self->vel2 < 0){
			//si es en reversa
			mcpwm_generator_set_force_level(self->gen_der_a, 0, true);
			mcpwm_generator_set_force_level(self->gen_der_b, -1, true);
			mcpwm_comparator_set_compare_value(self->cmpr_der, abs(self->vel2));
		}else if(self->vel2 > 0){
			//si es hacia adelante
			mcpwm_generator_set_force_level(self->gen_der_a, -1, true);
			mcpwm_generator_set_force_level(self->gen_der_b, 0, true);
			mcpwm_comparator_set_compare_value(self->cmpr_der, self->vel2);
		}else{
			//si debe frenar
			mcpwm_generator_set_force_level(self->gen_der_a, 1, true);
			mcpwm_generator_set_force_level(self->gen_der_b, 1, true);
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

//metodo que desbloquea los motores despues de los fallos
void ControlMotores::desbloqueo(){
	//se saltar el freno
	saltarFreno(true);

	//se desblouquean los operadores de ambos lados para ambos fallos
	mcpwm_operator_recover_from_fault(oper_izq, fallo_a);
	mcpwm_operator_recover_from_fault(oper_der, fallo_a);
	mcpwm_operator_recover_from_fault(oper_izq, fallo_b);
	mcpwm_operator_recover_from_fault(oper_der, fallo_b);
}

void ControlMotores::saltarFreno(bool estado){
	//se crea una variable de configuración de la accion
	mcpwm_gen_brake_event_action_t accion = {};
	accion.direction = MCPWM_TIMER_DIRECTION_UP;
	accion.brake_mode = MCPWM_OPER_BRAKE_MODE_OST;

	//se evalua si se deb saltar el freno
	if(estado){
		//si se desea saltar el freno, se configura la acción para que no afecte a los generadores
		accion.action = MCPWM_GEN_ACTION_KEEP;
	}else{
		//si no se desea saltar el freno, se configura la acción para que active el freno
		accion.action = MCPWM_GEN_ACTION_HIGH;
	}

	//aplicamos la accion
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_izq_a, accion));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_izq_b, accion));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_der_a, accion));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_der_b, accion));
}

//metodo que para el robot
void ControlMotores::alto(){
	velocidad(0, 0, false);
}

//metodo que avanza en direccion a
void ControlMotores::dir_a(){
	desbloqueo();
	velocidad(vel_nI, vel_nD, false);
}

//metodo que avanza en direccion b
void ControlMotores::dir_b(){
	desbloqueo();
	velocidad(-vel_nI, -vel_nD, false);

}

//metodo de ataque en direccion a izquierda
void ControlMotores::ataque_ai(){
	velocidad(vel_aI, vel_aD, true);
}

//metodo de ataque en direccion b izquierda
void ControlMotores::ataque_bi(){
	velocidad(-vel_aI, -vel_aD, true);
}

//metodo de ataque en direccion a derecha
void ControlMotores::ataque_ad(){
	//se invierte la direccion de las velocidades para efectuar el giro a la derecha
	velocidad(vel_aD, vel_aI, true);
}

//metodo de ataque en direccion b derecha
void ControlMotores::ataque_bd(){
	//se invierte la direccion de las velocidades para efectuar el giro a la derecha
	velocidad(-vel_aD, -vel_aI, true);
}

//metodo de velocidad maxima en direccion a
void ControlMotores::max_a(){
	velocidad(vel_mI, vel_mD, false);
}

//metodo de velocidad maxima en direccion b
void ControlMotores::max_b(){
	velocidad(-vel_mI, -vel_mD, false);
}

//metodo de ataque pronunciado en direccion a, a la izquierda
void ControlMotores::pronunciado_ai(){
	velocidad(vel_pI, vel_pD, true);
}

//metodo de ataque pronunciado en direccion b, a la izquierda
void ControlMotores::pronunciado_bi(){
	velocidad(-vel_pI, -vel_pD, true);
}

//metodo de ataque pronunciado en direccion a, a la derecha
void ControlMotores::pronunciado_ad(){
	//se invierte la direccion de las velocidades para efectuar el giro a la derecha
	velocidad(vel_pD, vel_pI, true);
}

//metodo de ataque pronunciado en direccion b, a la derecha
void ControlMotores::pronunciado_bd(){
	//se invierte la direccion de las velocidades para efectuar el giro a la derecha
	velocidad(-vel_pD, -vel_pI, true);
}

//metodo de giro
void ControlMotores::giro(){
	velocidad(vel_gI, vel_gD, false);
}

//metodo de evasion frontal
void ControlMotores::evasion_a(){
	velocidad(-vel_eI, -vel_eD, false);
}

//metodo de evasion trasera
void ControlMotores::evasion_b(){
	velocidad(vel_eI, vel_eD, false);
}

//metodo que inicializa y prepara todo
void ControlMotores::begin() {
	//se leen los valores del nvs
    nvsLeer();

	//se configura el timer
    mcpwm_timer_config_t timer_config = {};
    timer_config.group_id = 0;
    timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    timer_config.resolution_hz = freq;
    timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
    timer_config.period_ticks = period;

	//se crea el timer
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

	//se configura el operador
    mcpwm_operator_config_t operator_config = {};
    operator_config.group_id = 0;
    operator_config.flags.update_gen_action_on_tez = true;

	//se crean los operadores
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_izq));
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_der));

	//se conectan los operadores al timer
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_izq, timer));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_der, timer));

	//se configura el comparador
    mcpwm_comparator_config_t comparator_config = {};
    comparator_config.flags.update_cmp_on_tez = true;

	//se crean los comparadores
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_izq, &comparator_config, &cmpr_izq));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_der, &comparator_config, &cmpr_der));
	ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_izq, 0));
	ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_der, 0));

	//se configuran los generadores
    mcpwm_generator_config_t generator_config = {};

	//se crean los generadores y se asignan a los pines correspondientes del lado izquierdo
   	generator_config.gen_gpio_num = mot[0];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_izq, &generator_config, &gen_izq_a));
    generator_config.gen_gpio_num = mot2[0];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_izq, &generator_config, &gen_izq_b));

	//se crean los generadores y se asignan a los pines correspondientes del lado derecho
    generator_config.gen_gpio_num = mot[1];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_der, &generator_config, &gen_der_a));
    generator_config.gen_gpio_num = mot2[1];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_der, &generator_config, &gen_der_b));

	// Configuración de la acción de los generadores en eventos del timer
	mcpwm_gen_timer_event_action_t accion_timer = {};
	accion_timer.direction = MCPWM_TIMER_DIRECTION_UP;
	accion_timer.event = MCPWM_TIMER_EVENT_EMPTY;
	accion_timer.action = MCPWM_GEN_ACTION_HIGH;

	//se aplica la acción de los generadores en eventos del timer
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_izq_a, accion_timer));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_izq_b, accion_timer));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_der_a, accion_timer));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_der_b, accion_timer));

	//configuración de la acción de los generadores en eventos del comparador
	mcpwm_gen_compare_event_action_t accion_comparator = {};

	// Configuración de la acción de los generadores en eventos del comparador
	accion_comparator.direction = MCPWM_TIMER_DIRECTION_UP;
	accion_comparator.comparator = cmpr_izq;
	accion_comparator.action = MCPWM_GEN_ACTION_LOW;

	//se aplica la acción de los generadores en eventos del comparador
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_izq_a, accion_comparator));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_izq_b, accion_comparator));

	//se cambia la acción para el lado derecho
	accion_comparator.comparator = cmpr_der;

	//se aplica la acción de los generadores en eventos del comparador para el lado derecho
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_der_a, accion_comparator));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_der_b, accion_comparator));

#ifdef CONFIG_IDF_TARGET_ESP32S3
	// Configuración de falla para el sensor TCRT
    mcpwm_gpio_fault_config_t fault_config = {};
    fault_config.group_id = 0;
    fault_config.gpio_num = TCRT_1;
    fault_config.intr_priority = 5;
    fault_config.flags.active_level = 1;
    fault_config.flags.pull_up = 0;
    fault_config.flags.pull_down = 0;

	// Se crea el manejador de falla para el sensor TCRT de la direccion a
    ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&fault_config, &fallo_a));

	// Se crea el manejador de falla para el sensor TCRT de la direccion b
	fault_config.gpio_num = TCRT_2;
	ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&fault_config, &fallo_b));

	// Configuración de freno para la falla del sensor TCRT en direccion a
    mcpwm_brake_config_t freno_config = {};
    freno_config.fault = fallo_a;
    freno_config.brake_mode = MCPWM_OPER_BRAKE_MODE_OST;

	// Se asigna la acción de freno a ambos operadores en caso de falla del sensor TCRT en direccion a
    ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper_izq, &freno_config));
    ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper_der, &freno_config));

	// Se cambia la configuración de freno para la falla del sensor TCRT en direccion b
	freno_config.fault = fallo_b;

	// Se asigna la acción de freno a ambos operadores en caso de falla del sensor TCRT en direccion b
	ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper_izq, &freno_config));
	ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper_der, &freno_config));

	// Configuración de la acción de freno para los generadores en caso de evento de freno
    mcpwm_gen_brake_event_action_t accion_freno = {};
    accion_freno.direction = MCPWM_TIMER_DIRECTION_UP;
    accion_freno.brake_mode = MCPWM_OPER_BRAKE_MODE_OST;
    accion_freno.action = MCPWM_GEN_ACTION_HIGH;

	// Se asigna la acción de freno a los generadores en caso de evento de freno
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_izq_a, accion_freno));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_izq_b, accion_freno));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_der_a, accion_freno));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_der_b, accion_freno));
#endif

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    alto();

	//se crea la tarea de la rampa
	xTaskCreateStaticPinnedToCore(tareaRampa, "TareaRampa", sizeof(stackMotores), (void*)this, 5, stackMotores, &tcbMotores, 0);
}

void ControlMotores::controlador(int accion){
	//se estable o restablece al freno dependiendo de la accion
	switch(accion){
		case 0:
			alto();
			break;
		case 1:
			dir_a();
			break;
		case 2:
			dir_b();
			break;
		case 3:
			evasion_a();
			break;
		case 4:
			evasion_b();
			break;
		case 5:
			ataque_ai();
			break;
		case 6:
			ataque_bi();
			break;
		case 7:
			ataque_ad();
			break;
		case 8:
			ataque_bd();
			break;
		case 9:
			pronunciado_ai();
			break;
		case 10:
			pronunciado_bi();
			break;
		case 11:
			pronunciado_ad();
			break;
		case 12:
			pronunciado_bd();
			break;
		case 13:
			max_a();
			break;
		case 14:
			max_b();
			break;
		case 15:
			giro();
			break;
	}
}

void ControlMotores::nvsLeer(){
	//constructor de nvs con el namespace de motores
	Nvs nvs("motores");

	//se aplica cada variable
	tRam = nvs.leer("tiempo_rampa", tRam);
	vel_nI = nvs.leer("velocidad_nI", vel_nI);
	vel_nD = nvs.leer("velocidad_nD", vel_nD);
	vel_aI = nvs.leer("velocidad_aI", vel_aI);
	vel_aD = nvs.leer("velocidad_aD", vel_aD);
	vel_mI = nvs.leer("velocidad_mI", vel_mI);
	vel_mD = nvs.leer("velocidad_mD", vel_mD);
	vel_pI = nvs.leer("velocidad_pI", vel_pI);
	vel_pD = nvs.leer("velocidad_pD", vel_pD);
	vel_gI = nvs.leer("velocidad_gI", vel_gI);
	vel_gD = nvs.leer("velocidad_gD", vel_gD);
	vel_eI = nvs.leer("velocidad_eI", 950);
	vel_eD = nvs.leer("velocidad_eD", 200);
}

//envia los datos a la telemetria
void ControlMotores::velocidades(int16_t* v1, int16_t* v2, int16_t* vo1, int16_t* vo2){
	*v1 = vel1;
	*v2 = vel2;
	*vo1 = vel1_obj;
	*vo2 = vel2_obj;
}
