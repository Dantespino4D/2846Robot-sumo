#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "ControlMotores.h"
#include "rom/ets_sys.h"

ControlMotores::ControlMotores(gpio_num_t _pwm_1, gpio_num_t _pwm_2, gpio_num_t motA, gpio_num_t motB):
// Valores de configuracion pwm
    freq(5000),
    solut(LEDC_TIMER_8_BIT),
    pwmC_1(LEDC_CHANNEL_0),
    pwmC_2(LEDC_CHANNEL_1),

    // Pines pwm
    pwm_1(_pwm_1),
    pwm_2(_pwm_2),

	//pines de los motores
	mot{motA, motB}
{}

//estblecer velocidad
void ControlMotores::velocidad(int vel_1, int vel_2){
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_1, vel_1);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwmC_1);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwmC_2, vel_2);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwmC_2);
}

void ControlMotores::alto(){
	gpio_set_level(mot[0], 0);
  	gpio_set_level(mot[1], 0);
	velocidad(0, 0);
	ets_delay_us(100);
}

void ControlMotores::dir_a(){
	alto();
  	gpio_set_level(mot[0], 1);
  	gpio_set_level(mot[1], 1);
	velocidad(255, 255);
}

void ControlMotores::dir_b(){
	alto();
	gpio_set_level(mot[0], 0);
  	gpio_set_level(mot[1], 0);
	velocidad(255, 255);

}

void ControlMotores::giro(){
	alto();
	gpio_set_level(mot[0], 0);
  	gpio_set_level(mot[1], 1);
	velocidad(255, 255);
}

void ControlMotores::begin(){
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
		.gpio_num = pwm_1,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = pwmC_1,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_1);

	//configuracion canal 2
	ledc_channel_config_t ledc_channel_2 = {
		.gpio_num = pwm_2,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = pwmC_2,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.duty = 0,
		.hpoint = 0,
		.flags = 0
	};
	ledc_channel_config(&ledc_channel_2);
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
			giro();
			break;
	}
}
