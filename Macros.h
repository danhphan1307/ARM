/*
 * Macros.h
 *
 *  Created on: 9.10.2016
 *      Author: abdullai
 */

#ifndef MACROS_H_
#define MACROS_H_
#include "DigitalIoPin.h"
#include "board.h"

#endif /* MACROS_H_ */



#define DLY1MS (1/portTICK_PERIOD_MS)
#define DLY20MS (20/portTICK_PERIOD_MS)
#define DLY5MS (5/portTICK_PERIOD_MS)
#define DLY1SEC (1000/portTICK_PERIOD_MS)


//NEW MACROS

//PWM FOR LASER ?

/*
#define Laser DigitalIoPin(9,DigitalIoPin::output, true)

//Consult with Keijo
//PWM FOR PEN !?
#define Pen DigitalIoPin(2,DigitalIoPin::output)


#define SW1 DigitalIoPin(10,DigitalIoPin::pullup,true)
#define SW2 DigitalIoPin(11,DigitalIoPin::pullup,true)
#define SW3 DigitalIoPin(12,DigitalIoPin::pullup,true)


*/

//A3 layout (297X420)
//A4 layout (210X297)
//calculated in mm
#define xLength 347
#define yLength 310

//the actual max is

//327
//290
