#include "Spi.h"
#include "pines.h"
#include "driver/spi_master.h"
#include <string.h>
#include "protocolo/sumo_protocol.h"

#define TAG "spi"

Spi::Spi(){}

void Spi::begin() {
    spi_bus_config_t spibus = {};
    spibus.mosi_io_num = SPI_MOSI;
    spibus.miso_io_num = SPI_MISO;
    spibus.sclk_io_num = SPI_CLK;
    spibus.quadwp_io_num = -1;
    spibus.quadhd_io_num = -1;
    spibus.max_transfer_sz = MAX_PACKET_SIZE;

    spi_device_interface_config_t spistm32 = {};
    spistm32.clock_speed_hz = 10000000;
    spistm32.mode = 0;
    spistm32.spics_io_num = SPI_CS;
    spistm32.queue_size = 7;

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &spibus, SPI_DMA_CH_AUTO);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "error al configurar el bus SPI");
    }

    ret = spi_bus_add_device(SPI2_HOST, &spistm32, &stm32_handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "error al configurar el dispositivo SPI");
    }
}
