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


enum RIT_TYPE  {CALIBRATE, RUN}; // rit running type
#define DLY1MS (1/portTICK_PERIOD_MS)
#define DLY20MS (20/portTICK_PERIOD_MS)
#define DLY5MS (5/portTICK_PERIOD_MS)
#define DLY1SEC (1000/portTICK_PERIOD_MS)
#define DLY5SEC (5000/portTICK_PERIOD_MS)
#define TICK_RATE 1000000


#define MINSPEED 8000
#define MAXSPEED 20000


//calculated in 10 micro meters (um)
#define xLength 34700
#define yLength 31000

#define DefPPS 5000
//the actual max is

//327
//290


#endif /* MACROS_H_ */

