/*
 * DigitalIoPin.cpp
 *
 * Created on: 31.1.2016
 * Author: krl
 */
#include "DigitalIoPin.h"
#include "chip.h"
const int DigitalIoPin::dPort[] = { 0, 1, 1, 0, 0, 1, 0};
const int DigitalIoPin::dPin[] = { 17, 11, 9, 27, 28, 0, 24};
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

