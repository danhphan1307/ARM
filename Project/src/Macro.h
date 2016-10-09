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
#define DLY1SEC (1000/portTICK_PERIOD_MS)


//NEW MACROS
#define LimitSWXMin DigitalIoPin(0,DigitalIoPin::pullup,true)
#define LimitSWXMax DigitalIoPin(1,DigitalIoPin::pullup,true)

#define LimitSWYMin DigitalIoPin(3,DigitalIoPin::pullup,true)
#define LimitSWYMax DigitalIoPin(4,DigitalIoPin::pullup,true)


//PWM FOR LASER ?
#define Laser DigitalIoPin(9,DigitalIoPin::pullup,true)

//Consult with Keijo
//PWM FOR PEN !?
#define Pen DigitalIoPin(2,DigitalIoPin::output)

//MOTOR X
#define STEPX DigitalIoPin(7,DigitalIoPin::output)
#define DIRX DigitalIoPin(8,DigitalIoPin::output)


//MOTOR Y
#define STEPX DigitalIoPin(5,DigitalIoPin::output)
#define DIRX DigitalIoPin(6,DigitalIoPin::output)


#define SW1 DigitalIoPin(10,DigitalIoPin::pullup,true)
#define SW2 DigitalIoPin(11,DigitalIoPin::pullup,true)
#define SW3 DigitalIoPin(12,DigitalIoPin::pullup,true)




