#include "EstrategiaPrototipo.h"
#include "eventos.h"
#include "MaquinaEstados.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

void EstrategiaPrototipo::seleccion(MaquinaEstados* ctx) {
    //se reciben los bits de los sensores
	uint32_t noti = 0;
    noti = xEventGroupWaitBits(eventos, 0xFFFFFFFF, pdFALSE, pdFALSE, pdMS_TO_TICKS(10));

    //se verifica si se detecto algun sensor de color el que sea
    if(noti & MASK_COLOR){
        // si detecta el limite por sc_1
        if (noti & BIT_LIM_A) {
            ctx->modo = 0;
        }
        // si detecta el limite por sc_2
        else if (noti & BIT_LIM_B) {
            ctx->modo = 1;
        }
    //se verifica si hay alguna memoria del sensor de color
    }else if(ctx->memo_C != 0) {
        // si deja de detectar el limite por sc 1
        if (ctx->memo_C == 1) {
            ctx->modo = 2;
        }
        // si deja de detectar el limite por sc 2
        else if (ctx->memo_C == 2) {
            ctx->modo = 3;
        }
	}else if(ctx->stall) {
		ctx->modo = (ctx->memo_eva == 1) ? 4 : 5;
		return;
	}else if(ctx->detectarStall(ctx->corrienteA)) {
		ctx->stall = true;
		ctx->tempEva = (xTaskGetTickCount() * portTICK_PERIOD_MS);
		ctx->modo = (ctx->memo_eva == 1) ? 4 : 5;
		return;
    //se detecta si alguno de los sensores ultrasonicos fue activado
    }else if(noti & MASK_ULTRA) {
        // si detecta el robot por ultrasonico A
        if (noti & BIT_ULTRA_A) {
            ctx->modo = 6;
        }
        // si detecta el robot por ultrasonico B
        else if (noti & BIT_ULTRA_B) {
            ctx->modo = 7;
        }
    }else if(ctx->memo_TL != 0) {
        // si deja de detectar al robot por ultrasonico A
        if (ctx->memo_TL == 1) {
            ctx->modo = 8;
        }
        // si deja de detectar al robot por ultrasonico B
        else if (ctx->memo_TL == 2) {
            ctx->modo = 9;
        }
    }else{
        //memoria de avance de estrella
        if (ctx->memo_E) {
            ctx->modo = 10;
        }
        // si no detecta nada
        else{
            ctx->modo = 11;
        }
    }
}

void EstrategiaPrototipo::ejecucion(MaquinaEstados* ctx) {
    // variable del comandos
    int com = ALTO;

    // se guarda el tiempo
    unsigned long temp = (xTaskGetTickCount() * portTICK_PERIOD_MS);

    // ejecuta el estado
    switch (ctx->modo) {
        // detiene el movimiento y retrocede en direccion b
        case 0:
            com = HUIR_B;
            ctx->memo_C = 1;
            ctx->tempC = temp;
            break;
        // detiene el movimiento y retrocede en direccion a
        case 1:
            com = HUIR_A;
            ctx->memo_C = 2;
            ctx->tempC = temp;
            break;
        // continua avanzando en direccion b por un tiempo definido para alejarse del borde
        case 2:
            com = HUIR_B;
            break;
        // continua avanzando en direccion a por un tiempo definido para alejarse del borde
        case 3:
            com = HUIR_A;
            break;
		case 4:
			com = EVA_A;
			break;
		case 5:
			com = EVA_B;
			break;
        // avanza en direccion a
        case 6:
            com = ATAQUE_AI;
            ctx->memo_TL = 1;
            ctx->tempTL = temp;
            break;
        // avanza en direccion b
        case 7:
            com = ATAQUE_BI;
            ctx->memo_TL = 2;
            ctx->tempTL = temp;
            break;
        // avanza por un tiempo definido de 4 segundo en direccion a
        case 8:
            com = ATAQUE_AI;
            break;
        // avanza por un tiempo definido de 4 segundos en direccion b
        case 9:
            com = ATAQUE_BI;
            break;
        // da vueltas hasta encontrar el robot
        case 10:
            // aqui pondre la logica para que avanze formando la estrella
            if (temp - ctx->tempE1 >= (unsigned long)ctx->tiempo4) {
                ctx->memo_E = false; // ir a girar
                ctx->tempE2 = temp;
                com = GIRO;
            } else {
                com = DIR_A;
            }
            break;
        case 11:
            if (temp - ctx->tempE2 >= (unsigned long)ctx->tiempo5) {
                ctx->memo_E = true; // Termina giro → pasará a estado 8
                ctx->tempE1 = temp; // Reiniciar timer del avance
                com = DIR_A;
            } else {
                com = GIRO;
            }
            break;
        default:
            com = ALTO;
            break;
    }
    xTaskNotify(*(ctx->motr), com, eSetValueWithOverwrite);
}
