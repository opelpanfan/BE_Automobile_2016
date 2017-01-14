/*
 * pwm.c
 *
 *  Created on: Jan 14, 2017
 *      Author: Julien, Taha
 */

#include "MPC5604B.h" 
#include "pwm.h" 
#include "pin.h" 



/*
 * initialisation du bloc eMIOS0
 * configuration et activation du prescaler : 45 MHz --> 1 MHz
 */
void initEMIOS_0(void) {
 EMIOS_0.MCR.B.GPRE= 44; /* Divide 45 MHz sysclk by 44+1 = 45 for 1MHz eMIOS clk*/
 EMIOS_0.MCR.B.GPREN = 1; /* Enable prescaler to enable eMIOS clock */
 EMIOS_0.MCR.B.GTBE = 1; /* Enable global time base -- internal counters enabled*/
 EMIOS_0.MCR.B.FRZ = 1; /* Enable stopping channels when in debug mode */ 
}

/*
 * initialisation du canal 23, qui sert de driver pour le bus A --> synchro pour les canaux PWM
 * le canal 23 est configur� en mode compteur. Ce compteur d�borde toutes les 1 ms.
 */
void initEMIOS_0ch23(void) { 
 /* Contenu � donner au registre A du canal 23 : 999+1 = 1000 clocks of 1 �s period --> 1 msec */
 EMIOS_0.CH[23].CADR.R = 999; 
 /* config du mode : Modulus Counter Buffered (MCB) */ 
 EMIOS_0.CH[23].CCR.B.MODE = 0x50; 
 /* Use internal counter */
 EMIOS_0.CH[23].CCR.B.BSL = 0x3;
 /* Set channel prescaler to divide by 1 */ 
 EMIOS_0.CH[23].CCR.B.UCPRE=0; 
 /* Enable prescaler; uses default divide by 1. If not activated, no clock for the counter. */ 
 EMIOS_0.CH[23].CCR.B.UCPEN = 1; 
 /* Freeze channel counting when in debug mode */ 
 EMIOS_0.CH[23].CCR.B.FREN = 1; 
}

//initialisation du canal 21 : sortie PWM avec p�riode de 1 ms et rapport cyclique = 50 %.
//La position du leading edge est en d�but de p�riode.
//compteur fourni par le bus A. La p�riode et le rapport cyclique sont d�finies par la position
//des fronts montants et descendants (leading and trailing edges) donn�e par les registres A et B. 
void initEMIOS_0ch21(int x) { 
 /* register A: Leading edge when channel counter bus=0*/
 EMIOS_0.CH[21].CADR.R = 0; 
 /* register B: Trailing edge when channel counter bus=500*/ 
 EMIOS_0.CH[21].CBDR.R = x; 
 /* Use counter bus A (default) */ 
 EMIOS_0.CH[21].CCR.B.BSL = 0x0;
 EMIOS_0.CH[21].CCR.B.UCPRE=0;
 /* Polarity-leading edge sets output/trailing clears*/ 
 EMIOS_0.CH[21].CCR.B.EDPOL = 1;
 /* Mode is OPWM Buffered */ 
 EMIOS_0.CH[21].CCR.B.MODE = 0x60; 
}

//initialisation du canal 22 : sortie PWM avec p�riode de 1 ms et rapport cyclique = 25 %
//LA position du leading edge est � 80% (800/1000� de la p�riode.
//compteur fourni par le bus A. La p�riode et le rapport cyclique sont d�finies par la position
//des fronts montants et descendants (leading and trailing edges) donn�e par les registres A et B. 
void initEMIOS_0ch22(void) { 
 /* register A: Leading edge when channel counter bus=250*/
 EMIOS_0.CH[22].CADR.R = 0; 
 /* register B: Trailing edge when channel counter bus=500*/ 
 EMIOS_0.CH[22].CBDR.R = 500; 
 /* Use counter bus A (default) */ 
 EMIOS_0.CH[22].CCR.B.BSL = 0x0;
 EMIOS_0.CH[22].CCR.B.UCPRE=0;
 /* Polarity-leading edge sets output/trailing clears*/ 
 EMIOS_0.CH[22].CCR.B.EDPOL = 1;
 /* Mode is OPWM Buffered */ 
 EMIOS_0.CH[22].CCR.B.MODE = 0x60; 
}





/*
 * Configure pin en digital output. TODO � passer dans gpio.c
 */
void Configure_to_DO(uint32_t num_pin)
{
  SIU.PCR[num_pin].R = 0x0200;
} 

/*
 * Ecrit �tat du pin : 0 ou 1
 * si i>0 -> pin � 1
 * TODO passer dans gpio.c -> DigitalWrite(...)
 */
void Write_to_output(uint32_t num_pin, uint8_t i)  
  {
	if (i==0) {
		SIU.GPDO[num_pin].B.PDO = 0;
	} 
	else {
		SIU.GPDO[num_pin].B.PDO = 1;
	}  
  }

/*
 * Lit valeur pin.
 * TODO � passer dans gpio.c -> DigitalRead()
 */
uint8_t Read_input(uint32_t num_pin)
	{
		uint8_t Value = 0;     
        if(SIU.GPDI[num_pin].B.PDI == 1) {
        	Value = 1;
        }  
        return Value ;
	} 
 


