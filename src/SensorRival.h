#ifndef SENSORRIVAL_H
#define SENSORRIVAL_H
#include <Arduino.h>
#include <NewPing.h>

class SensorRival{
	private:
		// se crean objetos ojos_1 y ojos_2
		NewPing ojos_1;
		NewPing ojos_2;
		//variables de pines
		int trig_1;
		int trig_2;
		int echo_1;
		int echo_2;

		//limite de distancia
		int maxd;

	public:
		//costructor
		SensorRival(int _maxd, int _trig_1, int _echo_1, int _trig_2, int _echo_2);

		//metodos de verificacion
		bool ojos_1Verify();
		bool ojos_2Verify();
};

#endif // !SENSORRIVAL_H
