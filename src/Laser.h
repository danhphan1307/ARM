#include <stdint.h>
#include "chip.h"
#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "../freertos/inc/semphr.h"

#ifndef Laser_h
#define Laser_h

class Laser
{
public:
	Laser(int pin,int port);
	void Power(int power);
private:
	int pin;
	int port;
	int iCurrentPower;
};
#endif
