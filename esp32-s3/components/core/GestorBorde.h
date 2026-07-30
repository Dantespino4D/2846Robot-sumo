#ifndef GESTORBORDE_H
#define GESTORBORDE_H

#include "MaquinaEstados.h"

class GestorBorde {
	private:
		static void INT_Borde(void* arg);
	public:
		GestorBorde();
		void begin(MaquinaEstados* me);
};

#endif
