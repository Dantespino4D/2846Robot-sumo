#ifndef RGB_H
#define RGB_H

#include "driver/gpio.h"
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT   // Resolución (0-1023)
#define LEDC_FREQUENCY          (5000)              // Frecuencia 5 kHz

// --- Configuración Pin 16 ---
#define LEDC_PIN_16             GPIO_NUM_16
#define LEDC_CHANNEL_16         LEDC_CHANNEL_2

// --- Configuración Pin 17 ---
#define LEDC_PIN_17             GPIO_NUM_17
#define LEDC_CHANNEL_17         LEDC_CHANNEL_3

static inline void setup_pwm() {
    // 1. Configurar el Timer (Solo se hace una vez)
    // Este timer será compartido por ambos pines
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);


    // 2. Configurar el Canal 0 (para el Pin 16)
    ledc_channel_config_t ledc_channel_16 = {
        .gpio_num       = LEDC_PIN_16,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_16,
        .timer_sel      = LEDC_TIMER, // <-- Vinculado al Timer 0
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_16);


    // 3. Configurar el Canal 1 (para el Pin 17)
    ledc_channel_config_t ledc_channel_17 = {
        .gpio_num       = LEDC_PIN_17,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_17,
        .timer_sel      = LEDC_TIMER, // <-- Vinculado al mismo Timer 0
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_17);
}
static inline void set_16(uint32_t duty) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_16, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_16);
}
static inline void set_17(uint32_t duty) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_17, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_17);
}
// funcion que prende un led para detectar errores, etc.
static inline void rgb(int a, int b) {
    set_16(a);
    set_17(b);
}

#endif
