#include "MonitorSistema.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3
#include "esp_log.h"
#endif

MonitorSistema::MonitorSistema() {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    adc1_handle = NULL;
#endif
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
    return (float)raw * (3.3 / 4095.0) * 4.03;
#else
    return 0.0;
#endif
}

void MonitorSistema::termistores(float &t1, float &t2, float &t3, float &t4) {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    int r1, r2, r3, r4;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &r1);
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &r2);
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_5, &r3);
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &r4);
    t1 = (float)r1 * (3.3 / 4095.0);
    t2 = (float)r2 * (3.3 / 4095.0);
    t3 = (float)r3 * (3.3 / 4095.0);
    t4 = (float)r4 * (3.3 / 4095.0);
#else
    t1 = 0.0;
    t2 = 0.0;
    t3 = 0.0;
    t4 = 0.0;
#endif
}
