file(GLOB component_sources "*.cpp" "*.c")
idf_component_register(
    SRCS ${component_sources}
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio
)