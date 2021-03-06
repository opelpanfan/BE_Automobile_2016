/*
 * adc.c
 *
 *  Created on: Nov 18, 2016
 *      Author: Theo
 */

#include "adc.h"
#include "pin.h"
#include "MPC5604B.h"
#include "interrupt_number.h"


int setupADC()
{
	if(ADC.MSR.B.ADCSTATUS !=1) // if not in power-down mode
		{
			ADC.MCR.B.PWDN = 1; // request power-down mode. needed before configuring the ADC.
			while(ADC.MSR.B.ADCSTATUS != 1) {} // wait to be in power-down mode.
		}
	
	
	 ADC.MCR.B.OWREN = 1; // overwrite enabled
	 ADC.MCR.B.ADCLKSEL =1; // full clock speed
	 ADC.MCR.B.MODE = 0;   // one shot mode
	
	 
	 ADC.NCMR[0].R = 0; // Mask disable for all channels
	 ADC.NCMR[1].R = 0; // Mask disable for all channels
	 ADC.NCMR[2].R = 0; // Mask disable for all channels
	 ADC.JCMR[0].R = 0; // Mask disable for all channels
	 ADC.JCMR[1].R = 0; // Mask disable for all channels
	 ADC.JCMR[2].R = 0; // Mask disable for all channels
	 
	
	 
	 return 0;
}


//external channels not supported
int pinToADCChannel_and_Type(unsigned int pin, char * channel, char *type)
{
	 // precision channels
	  if(pin >= PB_4 && pin <=PB_7) 
		  {
		  *channel = (char) pin-PB_4; // ajout de (char) pour �viter un warning. 
		  *type = PRECISION_CHANNEL;
		  return 0;
		  }
	  else if (pin >= PD_0 && pin <=PD_11) 
		  {
		  *channel = (char)pin-PD_0;
		  *type = PRECISION_CHANNEL;
		  return 0;
		  }
	  
	  // normal channels
	  else if(pin >= PB_8 && pin <=PB_11) 
		  {
		  *channel = (char)pin-PB_8;
		  *type = STANDARD_CHANNEL;
		  return 0;
		  }
	  else if (pin >= PD_12 && pin <=PD_15)
		  {
		  *channel = (char)pin-PD_12;
		  *type = STANDARD_CHANNEL;
		  return 0;
		  }
	  else if (pin >= PF_0 && pin <=PF_7)
		  {
		  *channel = (char)pin-PF_0 ;
		  *type = STANDARD_CHANNEL;
		  return 0;
		  }
	
	  else return WRONG_PIN;
}


int pinToADCChannel(unsigned int pin)
{
	char channel, type;
	int result = pinToADCChannel_and_Type(pin, &channel, &type);
	return result;
}

void enableADC()
{
	 ADC.MCR.B.PWDN = 0; /* ADC enable */
}

void startConversion()
{
	ADC.MCR.B.NSTART =1;
	
}


int setupPin_ADC(unsigned int pin)
{
	
	if(ADC.MSR.B.ADCSTATUS !=1) // if not in power-down mode
		{
			ADC.MCR.B.PWDN = 1; // request power-down mode. needed before configuring the ADC.
			while(ADC.MSR.B.ADCSTATUS != 1) {} // wait to be in power-down mode.
		}
	
 
	  // enable channel corresponding to the pin for normal conversion.
	  
	  // precision channel
	  if(pin >= PB_4 && pin <=PB_7)  ADC.NCMR[0].R  |= (1<<(pin-PB_4)); 
	  else if (pin >= PD_0 && pin <=PD_11) ADC.NCMR[0].R |= (1<<(pin-PD_0)); 
	  
	  // normal channel
	  else if(pin >= PB_8 && pin <=PB_11) ADC.NCMR[1].R |= (1<<(pin-PB_8));
	  else if (pin >= PD_12 && pin <=PD_15) ADC.NCMR[1].R |= (1<<(pin-PD_12));
	  else if (pin >= PF_0 && pin <=PF_7) ADC.NCMR[1].R |= (1<<(pin-PF_0));
	
	  else return WRONG_PIN;
	  
	  SIU.PCR[pin].R = 0x2000; // pin  in analog mode 
	  return 0;
}


int setupPin_ADC_Interrupt(unsigned int pin, unsigned int interrupt_flag)
{
	// enable interrupt for the channel corresponding to the pin.
		  
	  // precision channel
	  if(pin >= PB_4 && pin <=PB_7)  ADC.CIMR[0].R  |= (1<<(pin-PB_4)); 
	  else if (pin >= PD_0 && pin <=PD_11) ADC.CIMR[0].R |= (1<<(pin-PD_0)); 
	  
	  // normal channel
	  else if(pin >= PB_8 && pin <=PB_11) ADC.CIMR[1].R |= (1<<(pin-PB_8));
	  else if (pin >= PD_12 && pin <=PD_15) ADC.CIMR[1].R |= (1<<(pin-PD_12));
	  else if (pin >= PF_0 && pin <=PF_7) ADC.CIMR[1].R |= (1<<(pin-PF_0));
	
	  else return WRONG_PIN;
	
	  ADC.IMR.R |= interrupt_flag; // set the ADC global interrupt flags
	  
	  return 0;
}



int analogRead(unsigned int pin)
{
	char channel;
	char channel_type;
	 
	if(pinToADCChannel_and_Type(pin,&channel, &channel_type) !=0 ) return WRONG_PIN; // check if the pin corresponds to a valid channel
	if(!(ADC.NCMR[channel_type].R & (1<<channel)))  return CHANNEL_DISABLED; // check if the channel is enabled in the NCMR register.
	
	
	ADC.MCR.B.NSTART =1;
	while(ADC.MSR.B.NSTART ==1) {} // wait the conversion to be completed
	
	if(channel_type == STANDARD_CHANNEL) channel +=32; // translate channel for standard channel
	
	if(ADC.CDR[channel].B.VALID ==1) // if data is valid
	{
		return ADC.CDR[channel].B.CDATA;
	}
	else return UNVALID_DATA;
		
}



/* All ADC channel share the same EOC ISR, be careful to check which channel has raised the ISR */
/* Priority : 15 highest, 0 lowest */
void attachInterrupt_ADC_EOC(INTCInterruptFn isr, unsigned char priority)
{
	INTC_InstallINTCInterruptHandler(isr, ADC_EOC, priority);
}


/* All the watchdog share the same ISR, be careful to check which watchdog has raised the ISR */
/* Priority : 15 highest, 0 lowest */
void attachInterrupt_ADC_WTCH(INTCInterruptFn isr, unsigned char priority)
{	
	INTC_InstallINTCInterruptHandler(isr, ADC_WD, priority);	                                    
}
 

int setupAnalogWatchdog(int pin, unsigned int high_threshold, int low_threshold, int watchdog)
{
	char channel;
	char channel_type;
	
	/* check all the arguments */
	if(pinToADCChannel_and_Type(pin,&channel, &channel_type) !=0 ) return WRONG_PIN; // check if the pin corresponds to a valid channel
	if(watchdog <0 || watchdog > 3) return WRONG_WATCHDOG;	
	
	ADC.TRC[watchdog].B.THRCH = channel; // set the channel
	ADC.THRHLR[watchdog].B.THRH = high_threshold; // set the threshold
	ADC.THRHLR[watchdog].B.THRL = low_threshold;
	
	
	if(high_threshold < ADC_MAX) ADC.WTIMR.R |= (0x10 << watchdog);/* enable ISR trigger on high threshold if the high threshold is < max */
	else ADC.THRHLR[watchdog].B.THRH = ADC_MAX;
	if(low_threshold > ADC_MIN)  ADC.WTIMR.R |= (0x1 << watchdog); /* enable ISR trigger on low  threshold if the low threshold is > min */
	else ADC.THRHLR[watchdog].B.THRL = 0;
	
	
	
	
	
	return 0;	
}

void startAnalogWatchdog(int watchdog)
{
	ADC.TRC[watchdog].B.THREN = 1;
}

void stopAnalogWatchdog(int watchdog)
{
	ADC.TRC[watchdog].B.THREN = 0;
}

int ADCChannelToCTUChannel(unsigned int adc_channel)
{
	if(adc_channel <= 15) return adc_channel;
	else if(adc_channel >31 && adc_channel <=47) return (adc_channel-16);
	
	return 0;
}

/* setup the adc channel to be used by the PIT3 channel through CTU. */
void setupChannel_CTU_trigger(unsigned int adc_channel)
{
	int channel;
	channel = ADCChannelToCTUChannel(adc_channel);
	
	ADC.MCR.B.CTUEN =1; // enable CTU to trigger conversion
	
	CTU.EVTCFGR[23].B.CHANNELVALUE = channel;
	//CTU.EVTCFGR[23].B.CLR_FLAG = 0;	//TODO find out what's for
	CTU.EVTCFGR[23].B.TM = 1;	// enable CTU trigger
}


