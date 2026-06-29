#ifndef MONITOR_SISTEMA_H
#define MONITOR_SISTEMA_H

#include "pines.h"
#ifdef CONFIG_IDF_TARGET_ESP32S3
#include "esp_adc/adc_oneshot.h"
#endif

class MonitorSistema {
private:
#ifdef CONFIG_IDF_TARGET_ESP32S3
    adc_oneshot_unit_handle_t adc1_handle;
#endif
    float ema_corriente;

public:
    MonitorSistema();
    void begin();
    float bateria();
    float corrienteStall();
};

#endif
