/*
 * DigitalIoPin.h
 *
 *  Created on: 31.1.2016
 *      Author: krl
 */

#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_

class DigitalIoPin {
public:
	enum pinMode {
		output,
		input,
		pullup,
		pulldown
	};
	DigitalIoPin(int arduinoPin, pinMode mode, bool invert = false);
	virtual ~DigitalIoPin();
	virtual bool read();
	void write(bool value);
private:
	const static int dPort[];
	const static int dPin[];
	int port;
	int pin;

};

#endif /* DIGITALIOPIN_H_ */
