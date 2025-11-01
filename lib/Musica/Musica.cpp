#include "Musica.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT
#define LEDC_DUTY_50_PERCENT    (512)

gpio_num_t buz;
int tt = 5000;
void pinMus(gpio_num_t pin){
// Configurar el Timer
    ledc_timer_config_t ledc_timer = {};
	ledc_timer.speed_mode       = LEDC_MODE;
    ledc_timer.timer_num        = LEDC_TIMER;
    ledc_timer.freq_hz          = 5000; // Frecuencia inicial (ej. 5kHz)
    ledc_timer.duty_resolution  = LEDC_DUTY_RES;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer);

    // Configurar el Canal
    ledc_channel_config_t ledc_channel = {};

	ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.channel        = LEDC_CHANNEL;
    ledc_channel.timer_sel      = LEDC_TIMER;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = pin;
	ledc_channel.duty           = 0;
	ledc_channel.hpoint         = 0;
    ledc_channel_config(&ledc_channel);
}
void nota(float no, float dur) {
	ledc_set_freq(LEDC_MODE, LEDC_TIMER, (uint32_t)no);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_50_PERCENT);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    vTaskDelay((TickType_t)(dur / portTICK_PERIOD_MS));

    ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0);  vTaskDelay(ESPA / portTICK_PERIOD_MS);
}
void sil(float dur){
	ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0);
	vTaskDelay(dur / portTICK_PERIOD_MS);
}

void adestes() {
  for (int x = 0; x < 2; x++) {
    nota(AB4, NEG);
    nota(AB4, BLA);
    nota(EB4, NEG);
    nota(AB4, NEG);
    nota(BB4, BLA);
    nota(EB4, BLA);
    nota(C5, NEG);
    nota(BB4, NEG);
    nota(C5, NEG);
    nota(DB5, NEG);
    nota(C5, BLA);
    nota(BB4, NEG);
    nota(AB4, NEG);
    nota(AB4, BLA);
    nota(G4, NEG);
    nota(F4, NEG);
    nota(G4, NEG);
    nota(AB4, NEG);
    nota(BB4, NEG);
    nota(C5, NEG);
    nota(G4, BLA);
    nota(F4, PUNT);
    nota(EB4, COR);
    nota(EB4, RED);
    for (int y = 0; y < 2; y++) {
      nota(EB5, BLA);
      nota(DB5, NEG);
      nota(C5, NEG);
      nota(DB5, BLA);
      nota(C5, BLA);
      nota(BB4, NEG);
      nota(C5, NEG);
      nota(AB4, NEG);
      nota(BB4, NEG);
      nota(G4, PUNT);
      nota(F4, COR);
      nota(EB4, NEG);
      nota(AB4, NEG);
      nota(AB4, NEG);
      nota(G4, NEG);
      nota(AB4, NEG);
      nota(BB4, NEG);
      nota(AB4, BLA);
      nota(EB4, NEG);
      nota(C5, NEG);
      nota(C5, NEG);
      nota(BB4, NEG);
      nota(C5, NEG);
      nota(DB5, NEG);
      nota(C5, BLA);
      nota(BB4, NEG);
      nota(C5, NEG);
      nota(DB5, NEG);
      nota(C5, NEG);
      nota(BB4, NEG);
      nota(AB4, NEG);
      nota(G4, BLA);
      nota(AB4, NEG);
      nota(DB5, NEG);
      nota(C5, BLA);
      nota(BB4, PUNT);
      nota(AB4, COR);
      nota(AB4, RED);
    }
  }
  vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void martinillo() {
  for (int x = 0; x < 2; x++) {
    nota(BB3, NEG);
    nota(C4, NEG);
    nota(D4, NEG);
    nota(BB3, NEG);
  }
  for (int x = 0; x < 2; x++) {
    nota(D4, NEG);
    nota(EB4, NEG);
    nota(F4, BLA);
  }
  for (int x = 0; x < 2; x++) {
    nota(F4, COR);
    nota(G4, COR);
    nota(F4, COR);
    nota(EB4, COR);
    nota(D4, NEG);
    nota(BB3, NEG);
  }
  for (int x = 0; x < 2; x++) {
    nota(C4, NEG);
    nota(F4, NEG);
    nota(BB3, BLA);
  }
}

void plantera() {
  for (int i = 0; i < 4; i++) {
    nota(E5, SEMI);
    nota(E5, SEMI);
    nota(E5, SEMI);
    nota(B4, SEMI);
    nota(C5, SEMI);
    nota(D5, COR);
    nota(D5, SEMI);
    nota(D5, SEMI);
    nota(D5, SEMI);
    nota(L4, SEMI);
    nota(B4, SEMI);
    nota(C5, COR);
    nota(C5, SEMI);
    nota(C5, SEMI);
    nota(C5, SEMI);
    nota(G4, SEMI);
    nota(L4, SEMI);
    nota(B4, COR);
    nota(B4, SEMI);
    nota(B4, SEMI);
    nota(B4, SEMI);
    nota(GB4, SEMI);
    nota(G4, SEMI);
    nota(L4, COR);
  }
}

void boss1() {
	nota(BB3, COR);
	sil(COR);
	nota(BB5, PUNT);
	nota(L5, PUNT);
	nota(AB5, BPUNT);
	nota(B5, NEG);
	nota(BB3, COR);
	nota(BB5, PUNT);
	nota(L5, PUNT);
	nota(BB3, COR);
	nota(BB4, COR);
	nota(L4, COR);
	nota(EB4, COR);
	nota(E4, COR);
	nota(DB4, COR);
	nota(D4, COR);
	nota(C4, COR);
	nota(BB3, COR);
	sil(COR);
	nota(BB5, PUNT);
	nota(L5, PUNT);
	nota(AB5, BPUNT);
	nota(G5, NEG);
	nota(BB3, COR);
	sil(COR);
	nota(BB5, PUNT);
	nota(L5, PUNT);
	nota(AB5, NEG);
	nota(G5, NEG);
	nota(L5, NEG);
	nota(B5, PUNT);
	for(int i = 0; i < 2; i++){
		nota(BB3, COR);
		nota(BB3, NEG);
		nota(BB3, COR);
		nota(C4, COR);
		nota(C4, NEG);
		nota(C4, COR);
		nota(DB4, COR);
		nota(DB4, NEG);
		nota(DB4, COR);
		nota(GB4, COR);
		nota(GB4, NEG);
		nota(G4, COR);
	}
	nota(F2, COR);
	nota(F2, COR);
	nota(DB3, SEMI);
	nota(C3, SEMI);
	nota(F3, SEMI);
	sil(SEMI);
	nota(C3, SEMI);
	nota(F3, SEMI);
	nota(GB4, SEMI);
	nota(F4, SEMI);
	sil(COR);
	nota(DB4, SEMI);
	nota(GB4, SEMI);
	nota(F4, CPPUNT);
	nota(C4, SEMI);
	nota(C3, SEMI);
	nota(F2, SEMI);
	sil(SPUNT);
	nota(F3, NEG);
	nota(BB3, NEG);
	nota(EB4, NEG);
	nota(AB4, NEG);
	nota(DB5, NEG);
	nota(GB5, PUNT);
	sil(COR);
	nota(G4, SEMI);
	nota(C5, SEMI);
	nota(F5, SEMI);
	nota(B5, SEMI);
	nota(BB5, NEG);
	for(int i = 0; i < 2; i++){
		nota(F4, COR);
		sil(COR);
	}
	for(int i = 0; i < 2; i++){
		nota(GB4, COR);
		sil(COR);
	}
	for(int i = 0; i < 3; i++){
		nota(E4, COR);
		sil(COR);
	}
	nota(BB1, COR);
	nota(BB1, COR);
	nota(DB2, COR);
	nota(BB1, COR);
	nota(BB3, COR);
	nota(F3, COR);
	nota(EB3, COR);
	nota(E3, COR);
	nota(BB1, COR);
	nota(F1, COR);
	nota(BB2, COR);
	nota(DB2, COR);
	nota(E2, COR);
	nota(C2, COR);
	nota(EB2, COR);
	nota(S1, COR);
	nota(BB1, COR);
	nota(BB1, COR);
	nota(DB2, COR);
	nota(BB1, COR);
	nota(BB4, COR);
	nota(F4, COR);
	nota(EB4, COR);
	nota(E4, COR);
	nota(BB3, COR);
	nota(DB4, COR);
	sil(NEG);
	nota(DB4, COR);
	nota(E4, COR);
	sil(NEG);
	nota(BB2, COR);
	nota(BB2, COR);
	nota(F3, COR);
	nota(BB2, COR);
	nota(BB3, COR);
	nota(F3, COR);
	nota(EB3, COR);
	nota(E3, COR);
	nota(BB2, COR);
	nota(F2, COR);
	nota(BB3, COR);
	nota(DB3, COR);
	nota(E3, COR);
	nota(C3, COR);
	nota(EB3, COR);
	nota(B2, COR);
	nota(BB3, COR);
	nota(BB3, COR);
	nota(DB4, COR);
	nota(BB3, COR);
	nota(BB4, COR);
	nota(F4, COR);
	nota(EB4, COR);
	nota(E4, COR);
	nota(BB2, RED);

}

void funp1() {
  nota(C5, COR);
  nota(C5, COR);
  nota(BB4, COR);
  nota(C5, COR);
  sil(COR);
  nota(G4, COR);
  sil(COR);
  nota(G4, COR);
  nota(C5, COR);
  nota(F5, COR);
  nota(E5, COR);
  nota(C5, COR);
  sil(BLA);
}

void funky() {
  funp1();
  funp1();
  for (int i = 0; i < 4; i++) {
    nota(C4, COR);
  }
  nota(E4, NEG);
  nota(E4, COR);
  nota(E4, COR);
  nota(G4, COR);
  nota(G4, NEG);
  nota(E5, PUNT);
  nota(D5, COR);
  nota(C5, NEG);
  funp1();
  for (int i = 0; i < 4; i++) {
    nota(C4, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(E4, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(G4, COR);
  }
  nota(E5, COR);
  nota(D5, COR);
  nota(C5, NEG);
  funp1();
  for (int i = 0; i < 4; i++) {
    nota(C5, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(BB4, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(L4, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(G4, COR);
  }
  nota(C4, COR);
  nota(C4, COR);
  nota(BB3, COR);
  nota(C4, COR);
  sil(COR);
  nota(G3, COR);
  sil(COR);
  nota(G3, COR);
  nota(C4, COR);
  nota(F4, COR);
  nota(E4, COR);
  nota(C4, COR);
  sil(BLA);
  nota(G4, COR);
  nota(G4, COR);
  nota(G4, NEG);
  nota(F4, COR);
  nota(F4, COR);
  nota(F4, NEG);
  nota(E4, COR);
  nota(E4, COR);
  nota(E4, NEG);
  nota(E4, NEG);
  nota(E4, NEG);
  funp1();
  funp1();
  for (int i = 0; i < 4; i++) {
    nota(C4, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(BB3, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(L3, COR);
  }
  for (int i = 0; i < 4; i++) {
    nota(G3, COR);
  }
  nota(F3, NEG);
  sil(NEG);
  sil(BLA);
  funp1();
  funp1();
  nota(F4, COR);
  nota(F4, COR);
  nota(EB4, COR);
  nota(F4, COR);
  sil(COR);
  nota(C4, COR);
  sil(COR);
  nota(C4, COR);
  nota(F4, COR);
  nota(BB4, COR);
  nota(L4, COR);
  nota(F4, COR);
  sil(BLA);
}
void prueba(){
	for(int i = 0; i < 60; i++){
		for(int j = 0; j < 8; j++){
			nota(Notas[i], COR);
		}
	}
}
