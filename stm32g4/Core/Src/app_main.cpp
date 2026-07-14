#include "app_main.h"
#include "Motores.h"
#include "Spi.h"
#include "comp.h"

extern Spi spi;
Motores mot;

void app_main(void){
	HAL_COMP_Start(&hcomp1);
	HAL_COMP_Start(&hcomp2);
	HAL_COMP_Start(&hcomp3);
	HAL_COMP_Start(&hcomp4);
	LL_DAC_Enable(DAC1,LL_DAC_CHANNEL_1);
	LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_2);
	mot.begin();
	spi.begin();
 while (1){
		__WFI();
	}
}
