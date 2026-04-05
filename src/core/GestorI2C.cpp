#include "GestorI2C.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "../actuadores/rgb.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3
    #define I2C_MASTER_SCL_IO GPIO_NUM_9
    #define I2C_MASTER_SDA_IO GPIO_NUM_8
#else
    #define I2C_MASTER_SCL_IO GPIO_NUM_22
    #define I2C_MASTER_SDA_IO GPIO_NUM_21
#endif

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

static const char* TAG = "GestorI2C";

GestorI2C::GestorI2C() : err(0) {}

void GestorI2C::begin() {
    conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    
    i2c_param_config(I2C_MASTER_NUM, &conf);
    esp_err_t res = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al instalar I2C: %s", esp_err_to_name(res));
        rgb(0, 1023);
    }
}

void GestorI2C::error() { err++; }

void GestorI2C::reset() { err = 0; }

bool GestorI2C::verify() {
    if (err >= ERR_MAX) {
        ESP_LOGE(TAG, "I2C tiene demasiados errores acumulados.");
        return false;
    }
    return true;
}

void GestorI2C::reinicio() {
    ESP_LOGW(TAG, "Reiniciando el bus I2C...");
    i2c_driver_delete(I2C_MASTER_NUM);

    gpio_config_t conf_o;
    conf_o.pin_bit_mask = (1ULL << I2C_MASTER_SCL_IO) | (1ULL << I2C_MASTER_SDA_IO);
    conf_o.mode = GPIO_MODE_OUTPUT_OD;
    conf_o.pull_up_en = GPIO_PULLUP_ENABLE;
    conf_o.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf_o.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&conf_o);

    gpio_set_level(I2C_MASTER_SDA_IO, 1);
    for (int i = 0; i < 9; i++) {
        gpio_set_level(I2C_MASTER_SCL_IO, 0);
        esp_rom_delay_us(10);
        gpio_set_level(I2C_MASTER_SCL_IO, 1);
        esp_rom_delay_us(10);
    }
    
    gpio_set_level(I2C_MASTER_SCL_IO, 0);
    gpio_set_level(I2C_MASTER_SDA_IO, 0);
    esp_rom_delay_us(10);
    gpio_set_level(I2C_MASTER_SCL_IO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(I2C_MASTER_SDA_IO, 1);
    esp_rom_delay_us(10);

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    reset();
    ESP_LOGI(TAG, "Bus I2C reiniciado con éxito.");
}

i2c_port_t GestorI2C::port() { return I2C_MASTER_NUM; }
