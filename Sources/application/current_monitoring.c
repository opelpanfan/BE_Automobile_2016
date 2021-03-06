/*
 * current_monitoring.c
 *
 *  Created on: Dec 21, 2016
 *      Author: Theo
 */

#include "current_monitoring.h"
#include "Buttons_management.h"
#include "define.h"


void adc_example()
{
	int result;
	
	result = setupADC();
	enableADC();
	
	while(1)
	{
		//result = analogRead(AD_PIN);
	}
}


void adc_wtch_isr()
{
	unsigned long flag = ADC.WTISR.R;
}

void adc_eoc_isr()
{
	//toggle LED_1
	if(SIU.GPDO[PE_4].R == 1) SIU.GPDO[PE_4].R = 0;
	else SIU.GPDO[PE_4].R = 1;
	
}
// configure PIT, CTU and ADC to trigger a conversion on AD_PIN every 100ms
void ctu_trigger_example()
{
	int result;
	init_LED();
	LED_off(1);
	LED_off(2);
	LED_off(3);
	LED_off(4);
	
	result = setupADC();
	
	setupPin_ADC_Interrupt(PB_4, EOCTU_MASK);	// enable EOC_CTU interrupt from pin PB_4 
	attachInterrupt_ADC_EOC(adc_eoc_isr, 8);	// set the ISR to be called for adc EOC interrupt
	
	enableADC();
	
	setupChannel_CTU_trigger(0); 	// link the PIT to the ADC channel 0 (PB4) through the CTU
	
	setupChannelPIT(3, 5000);  // use PIT_3, the only one to be linked to the CTU. period =100ms.
	startChannelPIT(3);	
}

void adc_watchdog_example()
{
	int result;
		
	result = setupADC();
	
	//result = setupAnalogWatchdog(AD_PIN, 700, 100, 0); // high_treshold =700, low = 100, first watchdog (0)
	attachInterrupt_ADC_WTCH(adc_wtch_isr, 7);
	startAnalogWatchdog(0);
	
	enableADC();
	
	
	while(1)
		{
			//result = analogRead(AD_PIN);
		}
}
