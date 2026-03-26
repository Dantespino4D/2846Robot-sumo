#include "EstrategiaEstandar.h"
#include "../core/MaquinaEstados.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//selecciona el estado
void EstrategiaEstandar::seleccion(MaquinaEstados* ctx) {
	uint32_t noti = 0; // INICIALIZADO A 0 PARA EVITAR BASURA
	xTaskNotifyWait(0x00, 0xFFFFFFFF, &noti, 0);

	// si detecta el limite por sc_1
	if (noti & (1 << 0)) {
		ctx->modo = 0;
	}
	// si detecta el limite por sc_2
	else if (noti & (1 << 1)) {
	 	 ctx->modo = 1;
	}
	// si deja de detectar el limite por sc 1
	else if (ctx->memo_C == 1) {
		ctx->modo = 2;
	}
	// si deja de detectar el limite por sc 2
	else if (ctx->memo_C == 2) {
		ctx->modo = 3;
	}
	//si detecta al robot por ToF1, ToF2 y ToF3
	else if((noti & (1 << 2)) && (noti & (1 << 3)) && (noti & (1 << 4))) {
		ctx->modo = 4;
	}
	//si detecta al robot por ToF1 y Tof2
	else if((noti & (1 << 2)) && (noti & (1 << 3))) {
		ctx->modo = 5;
	}
	//si detecta al robot por ToF1 y ToF3
	else if((noti & (1 << 2)) && (noti & (1 << 4))) {
		ctx->modo = 6;
	}
	//si detecta al robot por ToF2 y ToF3
	else if((noti & (1 << 3)) && (noti & (1 << 4))) {
		ctx->modo = 7;
	}
	// si detecta el robot por ToF1
	else if (noti & (1 << 2)) {
		ctx->modo = 8;
	}
	// si detecta el robot por ToF2
	else if (noti & (1 << 3)) {
		ctx->modo = 9;
	}
	//si detecta al robot por ToF3
	else if (noti & (1 << 4)) {
	  	ctx->modo = 10;
	}
	//si detecta al robot por ToF4, ToF5 y ToF6
	else if((noti & (1 << 5)) && (noti & (1 << 6)) && (noti & (1 << 7))) {
		ctx->modo = 11;
	}
	//si detecta al robot por ToF4 y ToF5
	else if((noti & (1 << 5)) && (noti & (1 << 6))) {
		ctx->modo = 12;
	}
	//si detecta al robot por ToF4 y ToF6
	else if((noti & (1 << 5) && (noti & (1 << 7)))){
		ctx->modo = 13;
	}
	//si detecta al robot por ToF5 y ToF6
	else if((noti & (1 << 6)) && (noti & (1 << 7))) {
		ctx->modo = 14;
	}
	// si detecta al robot por ToF4
	else if (noti & (1 << 5)) {
	  	ctx->modo = 15;
	}
	// si detecta al robot por ToF5
	else if (noti & (1 << 6)) {
		ctx->modo = 16;
	}
	//si detecta al robot por ToF6
	else if (noti & (1 << 7)) {
		ctx->modo = 17;
	}
	//si deja de detectar al robot por direccion AI_P a corto plazo
	else if (ctx->memo_TC == 1) {
		ctx->modo = 18;
	}
	//si deja de detectar al robot por direccion AD_P a corto plazo
	else if (ctx->memo_TC == 2) {
		ctx->modo = 19;
	}
	//si deja de detectar al robot por direccion BI_P a corto plazo
	else if (ctx->memo_TC == 3) {
		ctx->modo = 20;
	}
	//si deja de detectar al robot por direccion BD_P a corto plazo
	else if (ctx->memo_TC == 4) {
		ctx->modo = 21;
	}
	//si deja de detectar al robot por direccion AI_S a corto plazo
	else if (ctx->memo_TC == 5) {
		ctx->modo = 22;
	}
	//si deja de detectar al robot por direccion AD_S a corto plazo
	else if (ctx->memo_TC == 6) {
		ctx->modo = 23;
	}
	//si deja de detectar al robot por direccion BI_S a corto plazo
	else if (ctx->memo_TC == 7) {
		ctx->modo = 24;
	}
	//si deja de detectar al robot por direccion BD_S a corto plazo
	else if (ctx->memo_TC == 8) {
		ctx->modo = 25;
	}
	//si deja de detectar al robot de frente a corto plazo
	else if (ctx->memo_TC == 9) {
		ctx->modo = 26;
	}
	//si deja de detectar al robot por atras a corto plazo
	else if (ctx->memo_TC == 10) {
		ctx->modo = 27;
	}
	//si deja de detectar al robot por direccion AI a largo plazo
	else if (ctx->memo_TL == 1) {
	  	ctx->modo = 28;
	}
	//si deja de detectar al robot por direccion AD a largo plazo
	else if(ctx->memo_TL == 2) {
	  	ctx->modo = 29;
	}
	//si deja de detectar al robot por direccion BI a largo plazo
	else if (ctx->memo_TL == 3) {
		ctx->modo = 30;
	}
	//si deja de detectar al robot por direccion BD a largo plazo
	else if (ctx->memo_TL == 4) {
	  	ctx->modo = 31;
	}
	//memoria de avance de estrella
	else if (ctx->memo_E) {
		ctx->modo = 32;
	}
	// si no detecta nada
	else{
		ctx->modo = 33;
	}
}
