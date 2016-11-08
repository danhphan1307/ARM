#include "Servo.h"

#define	UPPER_LIMIT	3000
#define	MIDDLE		1500
#define	LOWER_LIMIT	500


Servo::Servo(int pin,int port)
{
	this->pin = pin;
	this->port = port;
	this->iCurrentDegree = UPPER_LIMIT/0.06;

	Chip_SCT_Init(LPC_SCT0);
	Chip_SCTPWM_Init(LPC_SCT0);

	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, pin, port);	//match servo motor with out
	LPC_SCT0->CONFIG |= (1 << 17);							// two 16-bit timers, auto limit
	LPC_SCT0->CTRL_U |= (71 << 5); 						    // SCT0 clock input is:
	// 72MHZ/(71+1) = 1MHz (1usec)
	LPC_SCT0->MATCH[0].L     = 20000-1;						// match 0 @ 20000/1MHz = 20000 usec (50Hz PWM freq)
	LPC_SCT0->MATCHREL[0].L  = 20000-1;

	LPC_SCT0->MATCH[1].L     = UPPER_LIMIT-1;				// match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->MATCHREL[1].L  = UPPER_LIMIT-1;
	LPC_SCT0->MATCH[1].H     = LOWER_LIMIT-1;
	LPC_SCT0->MATCHREL[1].H  = LOWER_LIMIT-1;

	LPC_SCT0->EVENT[0].STATE = 0xFFFF;						// event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL  = (1 << 12);                  // match 0 condition only
	LPC_SCT0->EVENT[1].STATE = 0xFFFF;                     // event 1 happens in all states
	LPC_SCT0->EVENT[1].CTRL  = (1 << 0)  | (1 << 12);      // match 1 condition only
	LPC_SCT0->OUT[0].SET = (1 << 0);                       // event 0 will set   SCT0_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1);                       // event 1 will clear SCT0_OUT0
	LPC_SCT0->CTRL_L    &= ~(1 << 2);                      // unhalt it by clearing bit 2 of CTRL reg
}

void Servo::Degree(int degree){
	int temp = degree/0.06;
	if(temp <= UPPER_LIMIT && temp >= LOWER_LIMIT){
		LPC_SCT0->MATCHREL[1].L= temp;
		iCurrentDegree = temp;
	}
}

int Servo::GetDegree(){
	return this->iCurrentDegree;
}
