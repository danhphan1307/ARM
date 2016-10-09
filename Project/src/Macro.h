/*
 * Macro.h
 *
 *  Created on: Sep 16, 2016
 *      Author: nguyenluong
 */

#ifndef MACRO_H_
#include "DigitalIoPin.h"
#define MACRO_H_


#define	LONG_DELAY	(20/portTICK_PERIOD_MS)
#define	SHORT_DELAY (1/portTICK_PERIOD_MS)
#define DELAY_1_SEC (1000/portTICK_PERIOD_MS)
#define	PIN_STEP	DigitalIoPin(6,DigitalIoPin::output)
#define PIN_DIR		DigitalIoPin(5,DigitalIoPin::output)

#define SWITCH_LEFT  DigitalIoPin(3,DigitalIoPin::pullup,true)
#define SWITCH_RIGHT  DigitalIoPin(4,DigitalIoPin::pullup,true)


#define BUTTON_LEFT  DigitalIoPin(0,DigitalIoPin::pullup,true)
#define BUTTON_RIGHT  DigitalIoPin(2,DigitalIoPin::pullup,true)




#endif /* MACRO_H_ */
