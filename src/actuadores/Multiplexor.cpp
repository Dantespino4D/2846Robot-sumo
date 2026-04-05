#include "Multiplexor.h"
#include "esp_log.h"

#define TCAADDR 0x70
static const char* TAG = "Multiplexor";

Multiplexor::Multiplexor() {}

bool Multiplexor::sel(i2c_port_t port, uint8_t canal) {
    if (canal > 7) return false;
    uint8_t data = 1 << canal;
    esp_err_t err = i2c_master_write_to_device(port, TCAADDR, &data, 1, pdMS_TO_TICKS(20));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error al seleccionar canal %d: %s", canal, esp_err_to_name(err));
        return false;
    }
    return true;
}
