#include "Laser.h"

#define	UPPER_LIMIT_LASER	255
#define	LOWER_LIMIT_LASER	0


Laser::Laser(int pin,int port)
{
	this->pin = pin;
	this->port = port;

	Chip_SCT_Init(LPC_SCT1);
	Chip_SCTPWM_Init(LPC_SCT1);


	LPC_SCT1->CONFIG |= (1 << 17);							// two 16-bit timers, auto limit
	LPC_SCT1->CTRL_U |= (71 << 5); 						    // SCT0 clock input is:
	// 72MHZ/(71+1) = 1MHz (1usec)

	Chip_SWM_MovablePortPinAssign(SWM_SCT1_OUT0_O, pin, port);	//match red

	Chip_SCTPWM_SetOutPin(LPC_SCT1, 1, 0); // Index 1 is SCT1_OUT0


	LPC_SCT1->MATCH[0].L    = UPPER_LIMIT_LASER-1;
	LPC_SCT1->MATCHREL[0].L  = UPPER_LIMIT_LASER-1;


	LPC_SCT1->EVENT[0].STATE = 0xFFFF;						// event 0 happens in all states
	LPC_SCT1->EVENT[0].CTRL  = (1 << 12);                  // match 0 condition only

	LPC_SCT1->EVENT[1].STATE = 0xFFFF;                     // event 1 happens in all states
	LPC_SCT1->EVENT[1].CTRL  = (1 << 0)  | (1 << 12);      // match 1 condition only

	LPC_SCT1->OUT[0].SET = (1 << 0);                       // event 0 will set   SCT0_OUT0
	LPC_SCT1->OUT[0].CLR = (1 << 1);                       // event 1 will clear SCT0_OUT0

	LPC_SCT1->CTRL_L    &= ~(1 << 2);                      // unhalt it by clearing bit 2 of CTRL reg

}

void Laser::Power(int power){

	if(power <= UPPER_LIMIT_LASER && power >= LOWER_LIMIT_LASER){
		LPC_SCT1->MATCHREL[1].L = 255-power;
	}
}
