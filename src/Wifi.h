#ifndef WIFI_H
#define WIFI_H
#define WIFI "INFINITUM288C_5"
#define CONTRASEÑA "2465364457"

class Wifi{
	private:
		void wifi();
		static void evento(void* arg, esp_event_base_t base, int32_t id, void* data);
	public:
		void begin();
};
#endif
