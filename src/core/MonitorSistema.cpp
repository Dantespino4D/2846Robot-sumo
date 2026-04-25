#include "MonitorSistema.h"
#include <cmath>

#ifdef CONFIG_IDF_TARGET_ESP32S3
#include "esp_log.h"
#endif

MonitorSistema::MonitorSistema() {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    adc1_handle = NULL;
#endif
    ema_corriente = 0.0f;
}

void MonitorSistema::begin() {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_5, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config);
#endif
}

float MonitorSistema::bateria() {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    int raw;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &raw);
    return (float)raw * (3.3f / 4095.0f) * 4.03f;
#else
    return 0.0f;
#endif
}

float MonitorSistema::corrienteStall() {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    int raw;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw);
    float v = (float)raw * (3.3f / 4095.0f);
    float i_inst = v / (0.1f * 10.0f);
    ema_corriente = (i_inst * 0.1f) + (ema_corriente * 0.9f);
    return ema_corriente;
#else
    return 0.0f;
#endif
}

float MonitorSistema::calcularCelsius(int raw) {
    if (raw <= 0) return 0.0f;
    if (raw >= 4095) return 99.0f;

    float fixed_resistor = 10000.0f;
    float r_ntc = fixed_resistor * ((float)raw / (4095.0f - (float)raw));

    float beta = 3950.0f;
    float r0 = 10000.0f;
    float t0 = 298.15f;

    float steinhart;
    steinhart = r_ntc / r0;
    steinhart = log(steinhart);
    steinhart /= beta;
    steinhart += 1.0f / t0;
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;

    return steinhart;
}

void MonitorSistema::termistores(float &t1, float &t2, float &t3, float &t4) {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    int r1, r2, r3, r4;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &r1);
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &r2);
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_5, &r3);
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &r4);

    t1 = calcularCelsius(r1);
    t2 = calcularCelsius(r2);
    t3 = calcularCelsius(r3);
    t4 = calcularCelsius(r4);
#else
    t1 = 0.0f;
    t2 = 0.0f;
    t3 = 0.0f;
    t4 = 0.0f;
#endif
}
