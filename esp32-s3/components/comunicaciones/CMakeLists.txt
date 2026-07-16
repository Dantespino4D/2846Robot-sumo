file(GLOB component_sources "*.cpp" "*.c")
idf_component_register(
    SRCS ${component_sources}
    INCLUDE_DIRS "." "../../.."
    REQUIRES esp_wifi esp_event esp_netif espressif__mqtt espressif__cjson esp_https_ota bootloader_support core configuracion sensores actuadores Musica driver espressif__mdns esp_driver_spi
)
