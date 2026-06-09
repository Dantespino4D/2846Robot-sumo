#include "Multiplexor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TCAADDR 0x70
static const char* TAG = "Multiplexor";

Multiplexor::Multiplexor() {}

bool Multiplexor::sel(GestorI2C* i2c, uint8_t canal) {
    if (canal > 7) return false;
    uint8_t data = 1 << canal;
    
    i2c_master_dev_handle_t dev = i2c->get_device(TCAADDR);
    if (!dev) return false;

    esp_err_t error = i2c_master_transmit(dev, &data, 1, pdMS_TO_TICKS(20));
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Error al seleccionar canal %d: %s", canal, esp_err_to_name(error));
        return false;
    }
    return true;
}
