file(GLOB component_sources "*.cpp" "*.c")
idf_component_register(
	SRCS "Estrategia1.cpp" "Estrategia2.cpp" "EstrategiaEstandar.cpp" "GestorBorde.cpp" "GestorI2C.cpp" "MaquinaEstados.cpp" "Nvs.cpp"
    INCLUDE_DIRS "."
	REQUIRES nvs_flash esp_driver_i2c esp_adc configuracion comunicaciones esp_driver_gpio driver ui esp_driver_ledc sensores actuadores
)
