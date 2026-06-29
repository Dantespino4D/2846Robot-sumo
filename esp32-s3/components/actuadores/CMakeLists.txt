file(GLOB component_sources "*.cpp" "*.c")
idf_component_register(
    SRCS ${component_sources}
    INCLUDE_DIRS "."
    REQUIRES esp_driver_mcpwm esp_driver_gpio esp_driver_i2c esp_driver_ledc configuracion driver ui core
)