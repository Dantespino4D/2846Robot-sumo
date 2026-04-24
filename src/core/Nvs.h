#ifndef NVS_H
#define NVS_H

#include <cstdint>
class Nvs{
	private:
		const char* nameE;
	public:
		Nvs(const char* _nameE);
		void guardar(const char* clave, int32_t valor);
		int32_t leer(const char* clave, int32_t valor);
		void guardarFloat(const char* clave, float valor);
		float leerFloat(const char* clave, float valor);
};

#endif
