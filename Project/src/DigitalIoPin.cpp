/*
 * DigitalIoPin.cpp
 *
 * Created on: 31.1.2016
 * Author: krl
 */
#include "DigitalIoPin.h"
#include "chip.h"
//									0	1	  2	   3	4	 5	  6		7	 8		9	 10	  11   12
//								    D2   D3   D4   D6   D7   D8   D9   D10   D11   D12   A0   A1   A2
const int DigitalIoPin::dPort[] ={  0,   0,   0,   1,   0,   0,   1,   0,    0,    0,    0,   1,   1 };
const int DigitalIoPin::dPin[] = { 29,   9,   10,   3,   0,   24,  0,   27,   28,   12,   8,   6,   8 };
#define RESERVED 0x80
DigitalIoPin::DigitalIoPin(int arduinoPin, pinMode mode, bool invert) {
	// ugly but at this point I don't want to throw exceptions
	if(arduinoPin < 0 || arduinoPin > 15) arduinoPin = 0;
	pin = dPin[arduinoPin];
	port = dPort[arduinoPin];
	if(mode == output) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
	}
	else {
		uint32_t pm = IOCON_DIGMODE_EN;
		if(invert) pm |= IOCON_INV_EN;
		if(mode == pullup) {
			pm |= IOCON_MODE_PULLUP;
		}
		else if(mode == pulldown) {
			pm |= IOCON_MODE_PULLDOWN;
		}
		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, pm);
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	}
}
DigitalIoPin::~DigitalIoPin() {
	// TODO Auto-generated destructor stub
}
bool DigitalIoPin::read() {
	return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}
void DigitalIoPin::write(bool value) {
	return Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
}
