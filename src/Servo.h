#include <stdint.h>
#include "chip.h"
#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "../freertos/inc/semphr.h"

#ifndef Servo_h
#define Servo_h

class Servo
{
public:
	Servo(int pin,int port);
	void Degree(int degree);
	int GetDegree();
private:
	int pin;
	int port;
	int iCurrentDegree;
};
#endif
