#include "driver/gpio.h"
#include "driver/ledc.h"
#include "ControlMotores.h"
#include "../core/Nvs.h"
#include <cstdint>
#include <cstdlib>

#ifdef CONFIG_IDF_TARGET_ESP32S3
    #define MODO_PWM LEDC_LOW_SPEED_MODE
#else
    #define MODO_PWM LEDC_HIGH_SPEED_MODE
#endif

ControlMotores::ControlMotores(gpio_num_t motA2, gpio_num_t motB2, gpio_num_t motA1, gpio_num_t motB1):
// Valores de configuracion pwm
    freq(20000),
    solut(LEDC_TIMER_10_BIT),
    pwmC_1(LEDC_CHANNEL_0),
    pwmC_2(LEDC_CHANNEL_1),
    pwmC_3(LEDC_CHANNEL_2),
    pwmC_4(LEDC_CHANNEL_3),

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
	//varaible de la rampa de aceleracion
	uint32_t fade;
	//condicional de la rampa de aceleracion
	if(ram){
		//aplica rampa de aceleracion
		fade = tRam;
	}else{
		//no aplica rampa de aceleracion
		fade = 0;
	}
	//verifica la direccion de los motores de los canales 1 y 3
	if(vel_1 > 0){
		//aplca la velocidad en direccin a
    	ledc_set_fade_with_time(MODO_PWM, pwmC_1, vel_1, fade);
    	ledc_set_fade_with_time(MODO_PWM, pwmC_3, 0, fade);
	}else if(vel_1 < 0){
		//aplca la velocidad en direccin b
    	ledc_set_fade_with_time(MODO_PWM, pwmC_1, 0, fade);
    	ledc_set_fade_with_time(MODO_PWM, pwmC_3, abs(vel_1), fade);
	}else{
		//frena el motor
    	ledc_set_fade_with_time(MODO_PWM, pwmC_1, 1023, fade);
		ledc_set_fade_with_time(MODO_PWM, pwmC_3, 1023, fade);
	}

	//verifica la direccion de los motores de los canales 2 y 4
	if(vel_2 > 0){
		//aplca la velocidad en direccin a
    	ledc_set_fade_with_time(MODO_PWM, pwmC_2, vel_2, fade);
    	ledc_set_fade_with_time(MODO_PWM, pwmC_4, 0, fade);
	}else if(vel_2 < 0){
		//aplca la velocidad en direccin b
    	ledc_set_fade_with_time(MODO_PWM, pwmC_2, 0, fade);
    	ledc_set_fade_with_time(MODO_PWM, pwmC_4, abs(vel_2), fade);
	}else{
		//frena el motor
    	ledc_set_fade_with_time(MODO_PWM, pwmC_2, 1023, fade);
		ledc_set_fade_with_time(MODO_PWM, pwmC_4, 1023, fade);
	}

	//aplica los cambios de velocidad
	ledc_fade_start(MODO_PWM, pwmC_1, LEDC_FADE_NO_WAIT);
    ledc_fade_start(MODO_PWM, pwmC_2, LEDC_FADE_NO_WAIT);
    ledc_fade_start(MODO_PWM, pwmC_3, LEDC_FADE_NO_WAIT);
    ledc_fade_start(MODO_PWM, pwmC_4, LEDC_FADE_NO_WAIT);
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
void ControlMotores::begin(){
	//se aplican datos de nvs
	nvsLeer();

  	// configuracion y asignacion de los pines pwm

	//configuracion del timer
	ledc_timer_config_t ledc_timer = {
		.speed_mode = MODO_PWM,
		.duty_resolution = solut,
		.timer_num = LEDC_TIMER_0,
		.freq_hz = (uint32_t)freq,
		.clk_cfg = LEDC_AUTO_CLK,
		.deconfigure = false
	};
	ledc_timer_config(&ledc_timer);

	//configuracion canal 1
	ledc_channel_config_t ledc_channel_1 = {
		.gpio_num = mot2[0],
		.speed_mode = MODO_PWM,
		.channel = pwmC_1,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_1);

	//configuracion canal 2
	ledc_channel_config_t ledc_channel_2 = {
		.gpio_num = mot2[1],
		.speed_mode = MODO_PWM,
		.channel = pwmC_2,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_2);

	//configuracion canal 3
	ledc_channel_config_t ledc_channel_3 = {
		.gpio_num = mot[0],
		.speed_mode = MODO_PWM,
		.channel = pwmC_3,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_3);

	//configuracion canal 4
	ledc_channel_config_t ledc_channel_4 = {
		.gpio_num = mot[1],
		.speed_mode = MODO_PWM,
		.channel = pwmC_4,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_4);
	//se instala la funcion para la rampa de aceleracion
	ledc_fade_func_install(0);
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
