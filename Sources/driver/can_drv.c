/*
 * can_drv.c
 *
 *  Created on: Jan 18, 2017
 *      Author: Gido
 */

#include "can_drv.h"
#include "MPC5604B.h"
#include "IntcInterrupts.h"
#include "config.h"
#include "define.h"
#include "interrupt_number.h"
#include "Buttons_management.h"


//FlexCAN1 initialisation
void initCAN1 (void) {
	
	uint8_t j;
	
	CAN_1.IMRL.B.BUF01M = 1;
	INTC_InstallINTCInterruptHandler(Interrupt_Rx_CAN1, CAN_BUF_00_03, CAN_BUF_ISR_PRIORITY);
		
	
	/* Put in Freeze Mode, local priority disable, enable only 8 message buffers, common ID filter */
	//CAN_1.MCR.B.SOFTRST = 1;
	//while (CAN_1.MCR.B.SOFTRST == 1) {}
	CAN_1.MCR.R = 0x5000003F;  
	while (CAN_1.MCR.B.FRZACK != 1) {}
	/* Configure for 8MHz OSC, 100KHz bit time */     
	CAN_1.CR.R = 0x04DB0006;     
	/* Disactivate all 64 message buffers */
	
	for (j=0; j<64; j++) {
		CAN_1.BUF[j].CS.B.CODE = 0;   
	} 
	/* MB 0 will be the TX buffer, so initialised with TX INACTIVE		*/
	CAN_1.BUF[0].CS.B.CODE = 8;     /* Message Buffer 0 set to TX INACTIVE */
	
	/* MB 1 will be RX buffer		*/
	CAN_1.BUF[1].CS.B.IDE = 0; 		/* MB 1 will look for a standard ID (11 bits) */
#ifdef BCM
	CAN_1.BUF[1].ID.B.STD_ID = ID_BCM; /* MB 1 will look for ID = 111 */
#endif
#ifdef DCM
	CAN_1.BUF[1].ID.B.STD_ID = ID_DCM; /* MB 1 will look for ID = 222 */
#endif
#ifdef IC
	CAN_1.BUF[1].ID.B.STD_ID = IC_DCM; /* MB 1 will look for ID = 333 */
#endif
	CAN_1.BUF[1].CS.B.CODE = 4; 	 /* MB 1 set to RX EMPTY*/  

	/*Common ID filtering: accept all bits if standard ID is used for matching*/
	CAN_1.RXGMASK.R = 0x1FFFFFFF; 

	/* Pin configuration		*/
	SIU.PCR[42].R = 0x0624;         /* MPC56xxB: Config port C10 as CAN1TX, open drain */
	SIU.PCR[43].R = 0x0100;         /* MPC56xxB: Configure port C11 as CAN1RX */
	SIU.PSMI[0].R = 0x01;           /* MPC56xxB: Select PCR 43 for CAN1RX Input */

	/* Leave Freeze mode			*/
	CAN_1.MCR.R = 0x0000003F;       /* Negate FlexCAN1 halt state for the 8 first message buffers */
}

void TransmitMsg(uint8_t * TxData, uint8_t length, uint16_t MsgID) {
	uint8_t	i;
	/* Assumption:  Message buffer CODE is INACTIVE --> done in initCAN1 */  
	CAN_1.BUF[0].CS.B.IDE = 0;           /* Use standard ID length */
	CAN_1.BUF[0].ID.B.STD_ID = MsgID;      /* Transmit ID */
	CAN_1.BUF[0].CS.B.RTR = 0;           /* Data frame, not remote Tx request frame */
	
	for (i=0;i<length;i++){
		CAN_1.BUF[0].DATA.B[i]=*TxData; /* Data to be transmitted */
	}
	for(i=length;i<8;i++){
		CAN_1.BUF[0].DATA.B[i] = 0;	/* On remplit le reste avec des 0 */
	}
	CAN_1.BUF[0].CS.B.LENGTH = length;
	CAN_1.BUF[0].CS.B.SRR = 1;           /* Tx frame (not required for standard frame)*/
	CAN_1.BUF[0].CS.B.CODE =0b1100;        /* Activate msg. buf. to transmit data frame */ 
}



//Receive a message on MB 1 with data ID=ID_BCM, ID_DCM or ID_IC
//Print 4 LSB bits of the first byte on PE4-PE7.
uint8_t ReceiveMsg(void) {
	vuint8_t j;
	vuint32_t dummy;
	vuint32_t result32;
	uint8_t RxCODE;
	uint8_t RxID;
	uint8_t RxLENGTH;
	uint8_t RxDATA[SIZE_BUFFER_CAN];
	uint8_t RxTIMESTAMP;

	
	//IFRL = IFLAG1 in Bolero datasheet.
	while (CAN_1.IFRL.B.BUF01I == 0) {}		/* Wait for CAN 1 MB 1 flag */
	RxCODE   = CAN_1.BUF[1].CS.B.CODE;		/* Read CODE, ID, LENGTH, DATA, TIMESTAMP */
	RxID     = CAN_1.BUF[1].ID.B.STD_ID;
	RxLENGTH = CAN_1.BUF[1].CS.B.LENGTH;
	for (j=0; j<RxLENGTH; j++) { 
		RxDATA[j] = CAN_1.BUF[1].DATA.B[j];
	}
	RxTIMESTAMP = CAN_1.BUF[1].CS.B.TIMESTAMP; 
	dummy = CAN_1.TIMER.R;                	/* Read TIMER to unlock message buffers */    
	CAN_1.IFRL.R = 0x00000002;           	/* Clear CAN 1 MB 0 flag */
	
	return RxDATA[0];
	
}
