file(GLOB component_sources "*.cpp" "*.c")
idf_component_register(
    SRCS ${component_sources}
    INCLUDE_DIRS "."
    REQUIRES esp_driver_rmt esp_driver_gpio esp_driver_i2c espp__vl53l configuracion core driver ui actuadores
)