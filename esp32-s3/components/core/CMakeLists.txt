file(GLOB component_sources "*.cpp" "*.c")
idf_component_register(
    SRCS ${component_sources}
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_driver_i2c esp_adc configuracion esp_driver_gpio driver ui esp_driver_ledc sensores
)