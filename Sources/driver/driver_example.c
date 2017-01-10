/*
 * driver_example.c
 *
 *  Created on: Jan 7, 2017
 *      Author: Theo
 */

#include "driver_example.h"


void adc_example()
{
	int result;
	
	result = setupADC();
	result =  setupPin_ADC(AD_PIN);
	enableADC();
	
	while(1)
	{
		result = analogRead(AD_PIN);
	}
}


void adc_wtch_isr()
{
	unsigned long flag = ADC.WTISR.R;
}

void adc_eoc_isr()
{
	/* clear interrupt flags */
	
	ADC.ISR.B.EOCTU = 1;  // in this example, the interrupt source can only be the EOC from a CTU triggered conversion.
	ADC.CEOCFR[0].B.EOC_CH0 =1; // here, we use only the channel 0.
	
	/* toggle LED_1 */
	if(SIU.GPDO[PE_4].R == 1) SIU.GPDO[PE_4].R = 0;
	else SIU.GPDO[PE_4].R = 1;
	
}
// configure PIT, CTU and ADC to trigger a conversion on AD_PIN every 100ms
void ctu_trigger_example()
{
	int result;
	init_LED();
	
	result = setupADC();
	result = setupPin_ADC(AD_PIN);
	
	setupPin_ADC_Interrupt(PB_4, EOCTU_MASK);	// enable EOC_CTU interrupt from pin PB_4 
	attachInterrupt_ADC_EOC(adc_eoc_isr, 8);	// set the ISR to be called for adc EOC interrupt
	
	enableADC();
	
	setupChannel_CTU_trigger(0); 	// link the PIT to the ADC channel 0 (PB4) through the CTU and enable CTU
	
	setupChannelPIT(3, 500);  // use PIT_3, the only one to be linked to the CTU. period =100ms.
	
	startChannelPIT(3);	
}


void adc_eoc_example()
{
	int result;
	init_LED();
	
	result = setupADC();
	result = setupPin_ADC(AD_PIN);
	
	setupPin_ADC_Interrupt(PB_4, EOC_MASK);	// enable EOC_CTU interrupt from pin PB_4 
	attachInterrupt_ADC_EOC(adc_eoc_isr, 8);	// set the ISR to be called for adc EOC interrupt
	enableADC();
	
	startConversion();
	
	
}

void adc_watchdog_example()
{
	int result;
		
	result = setupADC();
	result =  setupPin_ADC(AD_PIN);
	
	result = setupAnalogWatchdog(AD_PIN, 700, 100, 0); // high_treshold =700, low = 100, first watchdog (0)
	attachInterrupt_ADC_WTCH(adc_wtch_isr, 7);
	startAnalogWatchdog(0);
	
	enableADC();
	
	
	while(1)
		{
			result = analogRead(AD_PIN);
		}
}
