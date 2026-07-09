#include "app_main.h"
#include "Motores.h"
#include "Spi.h"
#include "comp.h"
#include "dac.h"

extern Spi spi;
Motores mot;

void app_main(void){
	HAL_COMP_Start(&hcomp1);
	HAL_COMP_Start(&hcomp2);
	HAL_COMP_Start(&hcomp3);
	HAL_COMP_Start(&hcomp4);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
	mot.begin();
	spi.begin();
 while (1){
		__WFI();
	}
}
