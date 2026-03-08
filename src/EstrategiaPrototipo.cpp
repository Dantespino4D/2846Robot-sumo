#include "EstrategiaPrototipo.h"
#include "MaquinaEstados.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void EstrategiaPrototipo::seleccion(MaquinaEstados* ctx) {
    uint32_t noti = 0; // INICIALIZADO A 0 PARA EVITAR BASURA
    xTaskNotifyWait(0x00, 0xFFFFFFFF, &noti, 10);

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
    // si detecta el robot por ojos 1
    else if (noti & (1 << 2)) {
        ctx->modo = 4;
    }
    // si detecta el robot por ojos 2
    else if (noti & (1 << 3)) {
        ctx->modo = 5;
    }
    // si deja de detectar al robot por ojos 1
    else if (ctx->memo_TL == 1) {
        ctx->modo = 6;
    }
    // si deja de detectar al robot por ojos 2
    else if (ctx->memo_TL == 2) {
        ctx->modo = 7;
    }
    // si no detecta nada
    else if (ctx->memo_E) {
        ctx->modo = 8;
    }
    else {
        ctx->modo = 9;
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
            com = DIR_B;
            ctx->memo_C = 1;
            ctx->tempC = temp;
            break;
        // detiene el movimiento y retrocede en direccion a
        case 1:
            com = DIR_A;
            ctx->memo_C = 2;
            ctx->tempC = temp;
            break;
        // continua avanzando en direccion b por un tiempo definido para alejarse del borde
        case 2:
            com = DIR_B;
            break;
        // continua avanzando en direccion a por un tiempo definido para alejarse del borde
        case 3:
            com = DIR_A;
            break;
        // avanza en direccion a
        case 4:
            com = ATAQUE_AI;
            ctx->memo_TL = 1;
            ctx->tempTL = temp;
            break;
        // avanza en direccion b
        case 5:
            com = ATAQUE_BI;
            ctx->memo_TL = 2;
            ctx->tempTL = temp;
            break;
        // avanza por un tiempo definido de 4 segundo en direccion a
        case 6:
            com = ATAQUE_AI;
            break;
        // avanza por un tiempo definido de 4 segundos en direccion b
        case 7:
            com = ATAQUE_BI;
            break;
        // da vueltas hasta encontrar el robot
        case 8:
            // aqui pondre la logica para que avanze formando la estrella
            if (temp - ctx->tempE1 >= (unsigned long)ctx->tiempo4) {
                ctx->memo_E = false; // ir a girar
                ctx->tempE2 = temp;
                com = GIRO;
            } else {
                com = DIR_A;
            }
            break;
        case 9:
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
