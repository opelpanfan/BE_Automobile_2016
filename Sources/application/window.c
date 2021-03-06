/*
 * window.c
 *
 *  Created on: Jan 11, 2017
 *      Author: Theo
 */
#ifdef DCM

#include "window.h"
#include "pwm.h"
#include "gpio.h"


MC33887_pinout window_HB = {IN1_W, IN2_W, D2_W, EN_W, FS_W, 0};

char button_up_irq_mask; 
char button_down_irq_mask;

int init_window()
{
	button_up_irq_mask = (1<<pin_to_EIRQ(BUTTON_UP));
	button_down_irq_mask = (1<<pin_to_EIRQ(BUTTON_DOWN));
	
	window_state = STOPPED;
	window_position = UNKNOW;
	setupChannelPIT(PIT_MODE_W, PIT_MODE_W_TEMPO);


	//init_HBridge(&window_HB); //TODO toujours utile avec gestion PWM ?

	setup_buttons_w();
	init_HBridge(&window_HB);
	cm_initialize();
	
	//pinMode(PA_0, OUTPUT); // IN2 for H-bridge of window
	//pinMode(PA_1, OUTPUT); // ENABLE for H-bridge of window
	//init_PWM_0();
}


void pit_wtch_tempo_isr()
{
	/* clear interrupt flag */
	PIT.CH[CM_PIT_WTCH_TEMPO].TFLG.B.TIF =1;
	
	stopChannelPIT(CM_PIT_WTCH_TEMPO);
	startChannelPIT(CM_PIT_CTU);
}


void buttons_w_isr()
{
	
	if((SIU.ISR.R & 0x1))  // if isr raised by PA_3
	{
		stop_HBridge(&window_HB); 
	}
	
	
	/* if isr raised by BUTTON_UP rising edge */
	if((SIU.ISR.R & button_up_irq_mask) && SIU.GPDI[BUTTON_UP].B.PDI) 
	{
		if(window_state == STOPPED) 
		{
			start_HBridge(&window_HB,SENS1); 
			window_state = UP;
			startChannelPIT(CM_PIT_WTCH_TEMPO);
			startChannelPIT(PIT_MODE_W);
		}
		else 
		{
			stop_HBridge(&window_HB);
			stop_PITs();
			window_state = STOPPED;
		}
		
		
	}
	
	
	/* if isr raised by BUTTON_DOWN rising edge */
	if((SIU.ISR.R & button_down_irq_mask) && SIU.GPDI[BUTTON_DOWN].B.PDI) 
	{
		if(window_state == STOPPED) 
		{ 
			start_HBridge(&window_HB, SENS2); 
			window_state = DOWN;
			startChannelPIT(CM_PIT_WTCH_TEMPO);
			startChannelPIT(PIT_MODE_W);
		}
		else
		{
			stop_HBridge(&window_HB);
			stop_PITs();
			window_state = STOPPED;
		}
	}
	
	/* if isr raised by BUTTON_UP falling edge  */
	if((SIU.ISR.R & button_up_irq_mask) && (SIU.GPDI[BUTTON_UP].B.PDI== 0) ) 
		{
			if(window_state == UP)
			{	
				//  check if PIT_MOD_W > 100ms
				stopChannelPIT(PIT_MODE_W);
				/* if more than 100 ms has elapsed since the motor has started => manual mode => stop the motor. Else automatic mode */
				//if(PIT.CH[PIT_MODE_W].CVAL.R < MODE_W_THRESHOLD) 
				if(PIT.CH[PIT_MODE_W].TFLG.B.TIF == 1)
				{	
					stop_HBridge(&window_HB);
					stop_PITs();
					window_state = STOPPED;
				}
			}
		}
	
	
	/* if isr raised by BUTTON_DOWN falling edge */
	if((SIU.ISR.R & button_down_irq_mask) && (SIU.GPDI[BUTTON_DOWN].B.PDI== 0)) 
		{
			if(window_state == DOWN)
			{
				//  check if PIT_MOD_W > 100ms
				stopChannelPIT(PIT_MODE_W);
				/* if more than 100 ms has elapsed since the motor has started => manual mode => stop the motor. Else automatic mode */
				//if(PIT.CH[PIT_MODE_W].CVAL.R < MODE_W_THRESHOLD)
				if(PIT.CH[PIT_MODE_W].TFLG.B.TIF == 1)
				{	
					stop_HBridge(&window_HB);
					stop_PITs();
					window_state = STOPPED;
				}
			}
		
		}
	
	/* clear interrupt flag */
		// clear all EIRQ0 isr
	SIU.ISR.R = 0x00FF;
	
	/* toggle LED_1 */
	if(SIU.GPDO[PE_4].R == 1) SIU.GPDO[PE_4].R = 0;
	else SIU.GPDO[PE_4].R = 1;
	
}

int setup_buttons_w()
{
	int result=0;
	
	pinMode(BUTTON_UP, INPUT);
	pinMode(BUTTON_DOWN, INPUT);
	
	result = setup_EIRQ_pin(BUTTON_UP, BOTH);
	result = setup_EIRQ_pin(BUTTON_DOWN, BOTH);
	attachInterrupt_EIRQ0(buttons_w_isr, EIRQ0_PRIORITY);
	
	return result;
}

void stop_PITs()
{
	stopChannelPIT(CM_PIT_CTU);
	stopChannelPIT(CM_PIT_WTCH_TEMPO);
	stopChannelPIT(PIT_MODE_W);
	PIT.CH[PIT_MODE_W].TFLG.B.TIF= 1 ; 
}



void window_up()
{
	digitalWrite(PA_1,1);
	digitalWrite(PA_0,0);   
	//initEMIOS_0ch21(900);
	start_PWM_0(21, 90); //Rapport cyclique de 90% � cause de PA_0 � 0
}

void window_down()
{
	digitalWrite(PA_1,1);
	digitalWrite(PA_0,1); 
	//initEMIOS_0ch21(300);
	start_PWM_0(21, 30); //Rapport cyclique de 70% � cause de PA_0 � 1
}

void window_stop()
{
	digitalWrite(PA_1,0);
	digitalWrite(PA_0,0); //Utile ?
}

#endif
