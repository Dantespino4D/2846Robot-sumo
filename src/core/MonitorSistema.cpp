#include "MonitorSistema.h"

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
    adc_oneshot_unit_init_cfg_t init_config1 = {};
    init_config1.unit_id = ADC_UNIT_1;
    init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {};
    config.atten = ADC_ATTEN_DB_12;
    config.bitwidth = ADC_BITWIDTH_DEFAULT;

    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
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
