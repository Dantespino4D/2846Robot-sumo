#ifndef OTA_H
#define OTA_H

class Ota{
	private:
		static void tareaOta(void *pvParameter);
	public:
		void ota(const char* url);
};

#endif
