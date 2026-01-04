#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "ControlMotores.h"
#include "Nvs.h"
#include "rom/ets_sys.h"

ControlMotores::ControlMotores(gpio_num_t motA2, gpio_num_t motB2, gpio_num_t motA1, gpio_num_t motB1):
// Valores de configuracion pwm
    freq(5000),
    solut(LEDC_TIMER_10_BIT),
    pwmC_1(LEDC_CHANNEL_0),
    pwmC_2(LEDC_CHANNEL_1),
    pwmC_3(LEDC_CHANNEL_2),
    pwmC_4(LEDC_CHANNEL_3),

	//velocidades por defecto
	vel_nI(700),
	vel_nD(700),
	vel_aI(950),
	vel_aD(800),
	vel_gI(950),
	vel_gD(800)
{
	//pines pwm
	mot2[0] = motA2;
    mot2[1] = motB2;
	//pines de los motores
    mot[0]  = motA1;
    mot[1]  = motB1;
}

//estblecer velocidad
void ControlMotores::velocidad(int vel_1, int vel_2){
	vel1 = vel_1;
	vel2 = vel_2;
	if(vel_1 > 0){
    	ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_1, vel_1);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_3, 0);
	}else if(vel_1 < 0){
    	ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_1, 0);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_3, abs(vel_1));
	}else if(vel_1 == 0){
    	ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_1, 1023);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_3, 1023);
	}

	if(vel_2 > 0){
    	ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_2, vel_2);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_4, 0);
	}else if(vel_2 < 0){
    	ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_2, 0);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_4, abs(vel_2));
	}else if(vel_2 == 0){
	    ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_2, 1023);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_4, 1023);
	}

	ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwmC_1);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwmC_2);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwmC_3);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwmC_4);
}

void ControlMotores::alto(){
	velocidad(0, 0);
	ets_delay_us(100);
}

void ControlMotores::dir_a(){
	//alto();
	velocidad(vel_nI, vel_nD);
}

void ControlMotores::dir_b(){
	//alto();
	velocidad(-vel_nI, -vel_nD);

}

void ControlMotores::ataque_a(){
	//alto();
	velocidad(vel_aI, vel_aD);
}

void ControlMotores::ataque_b(){
	//alto();
	velocidad(-vel_aI, -vel_aD);
}

void ControlMotores::giro(){
	//alto();
	velocidad(vel_gI, -vel_gD);
}

void ControlMotores::begin(){
	//se aplican datos de nvs
	nvsLeer();

  	// configuracion y asignacion de los pines pwm

	//configuracion del timer
	ledc_timer_config_t ledc_timer = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
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
		.speed_mode = LEDC_HIGH_SPEED_MODE,
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
		.speed_mode = LEDC_HIGH_SPEED_MODE,
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
		.speed_mode = LEDC_HIGH_SPEED_MODE,
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
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = pwmC_4,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_4);
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
			ataque_a();
			break;
		case 4:
			ataque_b();
			break;
		case 5:
			giro();
			break;
	}
}

void ControlMotores::nvsLeer(){
	//constructor de nvs con el namespace de motores
	Nvs nvs("motores");

	//se aplica cada variable
	vel_nI = nvs.leer("velocidad_nI", vel_nI);
	vel_nD = nvs.leer("velocidad_nD", vel_nD);
	vel_aI = nvs.leer("velocidad_aI", vel_aI);
	vel_aD = nvs.leer("velocidad_aD", vel_aD);
	vel_gI = nvs.leer("velocidad_gI", vel_gI);
	vel_gD = nvs.leer("velocidad_gD", vel_gD);
}

//envia los datos a la telemetria
void ControlMotores::velocidades(int16_t* v1, int16_t* v2){
	*v1 = vel1;
	*v2 = vel2;
}
