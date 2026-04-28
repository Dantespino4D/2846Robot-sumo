#include "driver/gpio.h"
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
    period(1023), // Ajustado a 1023 para coincidir con el rango de velocidad
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
	vel2(0)
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
	vel1 = vel_1;
	vel2 = vel_2;
}

//metodo que para el robot
void ControlMotores::alto(){
	velocidad(0, 0, false);
}

//metodo que avanza en direccion a
void ControlMotores::dir_a(){
	velocidad(vel_nI, vel_nD, false);
}

//metodo que avanza en direccion b
void ControlMotores::dir_b(){
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
    nvsLeer();

    mcpwm_timer_config_t timer_config = {};
    timer_config.group_id = 0;
    timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    timer_config.resolution_hz = freq;
    timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
    timer_config.period_ticks = period;

    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_operator_config_t operator_config = {};
    operator_config.group_id = 0;
    operator_config.flags.update_gen_action_on_tez = true;

    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_izq));
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_der));

    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_izq, timer));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_der, timer));

    mcpwm_comparator_config_t comparator_config = {};
    comparator_config.flags.update_cmp_on_tez = true;

    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_izq, &comparator_config, &cmpr_izq));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_der, &comparator_config, &cmpr_der));

    mcpwm_generator_config_t generator_config = {};

    generator_config.gen_gpio_num = mot[0];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_izq, &generator_config, &gen_izq_a));
    generator_config.gen_gpio_num = mot2[0];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_izq, &generator_config, &gen_izq_b));

    generator_config.gen_gpio_num = mot[1];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_der, &generator_config, &gen_der_a));
    generator_config.gen_gpio_num = mot2[1];
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_der, &generator_config, &gen_der_b));

#ifdef CONFIG_IDF_TARGET_ESP32S3
    mcpwm_gpio_fault_config_t fault_config = {};
    fault_config.group_id = 0;
    fault_config.gpio_num = TCRT_1;
    fault_config.intr_priority = 5;
    fault_config.flags.active_level = 1;
    fault_config.flags.pull_up = 0;
    fault_config.flags.pull_down = 0;

    mcpwm_fault_handle_t fallo_izquierdo;
    ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&fault_config, &fallo_izquierdo));

    mcpwm_brake_config_t freno_config = {};
    freno_config.fault = fallo_izquierdo;
    freno_config.brake_mode = MCPWM_OPER_BRAKE_MODE_OST;

    ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper_izq, &freno_config));
    ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper_der, &freno_config));

    mcpwm_gen_brake_event_action_t accion_freno = {};
    accion_freno.direction = MCPWM_TIMER_DIRECTION_UP;
    accion_freno.brake_mode = MCPWM_OPER_BRAKE_MODE_OST;
    accion_freno.action = MCPWM_GEN_ACTION_LOW;

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_izq_a, accion_freno));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_izq_b, accion_freno));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_der_a, accion_freno));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(gen_der_b, accion_freno));
#endif

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    alto();
}

void ControlMotores::controlador(int accion){
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
void ControlMotores::velocidades(int16_t* v1, int16_t* v2){
	*v1 = vel1;
	*v2 = vel2;
}
