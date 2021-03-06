#include "MPC5604B.h"
#include "Buttons_management.h"
#include "pin.h"
#include "adc.h"
#include "gpio.h"


#define BUTTON_LOCK PA_3

void init_speed_button()
{
    setupADC();
    setupPin_ADC(SPEED_SENSOR);
    enableADC();
}


uint16_t read_speed()  
{ 
	uint16_t speed= analogRead(SPEED_SENSOR)/32;
    return speed;
}


void init_LED()
{
	 SIU.PCR[PE_4].R = 0x0200;
	 SIU.PCR[PE_5].R = 0x0200;
	 SIU.PCR[PE_6].R = 0x0200;
	 SIU.PCR[PE_7].R = 0x0200;
}

void init_buttons()
{
	int error;
	
	 SIU.PCR[PE_0].R = 0x0100;
	 SIU.PCR[PE_1].R = 0x0100; 
	 SIU.PCR[PE_2].R = 0x0100;
	 SIU.PCR[PE_3].R = 0x0100;
	 
	 SIU.PCR[BUTTON_LOCK].R = 0x0100;
	 error = setup_EIRQ_pin(BUTTON_LOCK, FALLING);
}


uint8_t  button_lock_door()
{   
	uint8_t  B_door = 0;
	
	if(SIU.GPDI[PE_0].B.PDI == 0){
	   B_door = 1 ;
	}
    else  B_door = 1   ;
	return B_door;
}



uint8_t det_rain()
{  
	uint8_t  B_rain = 0;

    if(SIU.GPDI[PE_1].B.PDI == 0){
   		B_rain = 1 ;
   	}
   	else B_rain = 0;
   	return  B_rain;
}

uint8_t bat_min()
{    
	uint8_t  B_bat = 0;
    
    if(SIU.GPDI[PE_2].B.PDI == 0){
    	B_bat = 1;
    }
	else B_bat = 0;
  	return  B_bat ;
}


uint8_t bouton4()
{
	uint8_t  B = 0;
    
    if(SIU.GPDI[PE_3].B.PDI == 0){
    	B = 1;
    }
	else B = 0;
  	return  B ;
}


void LED_off(uint8_t numLED)
{
	switch(numLED)
	{
	case 1:
		SIU.GPDO[PE_4].R = 0;
		break;
	case 2:
		SIU.GPDO[PE_5].R = 0;
		break;
	case 3:
		SIU.GPDO[PE_6].R = 0;
		break;
	case 4:
		SIU.GPDO[PE_7].R = 0;
		break;
	}
}


void LED_on(uint8_t numLED)
{
	switch(numLED)
	{
	case 1:
		SIU.GPDO[PE_4].R = 1;
		break;
	case 2:
		SIU.GPDO[PE_5].R = 1;
		break;
	case 3:
		SIU.GPDO[PE_6].R = 1;
		break;
	case 4:
		SIU.GPDO[PE_7].R = 1;
		break;
	}
}

/*

void test_buttons_management()
{
	//disableWatchdog();
	//initModesAndClock();
	
	init_LED();
	init_buttons();
	
	while(1) {
		 if(button_lock_door() == 1) {
			LED_on(1); 
		 }
		 else LED_off(1);
		 
		 if(det_rain() == 1){
			 LED_on(2);
		 }
		 else LED_off(2);
		 
		 if(bat_min() == 1) {   
			 LED_on(3);
		 }
		 else LED_off(3);
		 
		 if(bouton4() == 1) {
			 LED_on(4);
		 }
		 else LED_off(4);
	}
}
*/
void test_speed()
{
	init_LED();
	init_buttons();
	init_speed_button();
	
	LED_on(2);
	LED_on(3);
	LED_on(4);
	
	while(1) {
		if(read_speed()>1) {
			LED_off(1);
		}
		else {
			LED_on(1);
		}
	}

}


