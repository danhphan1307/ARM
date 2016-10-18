/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>
#include <String>
//#include "DigitalIoPin.h"
#include <stdio.h>
#include <string.h>
#include "semphr.h"
#include "Macros.h"
#include "DigitalIoPin.h"
#include "Motor.h"
// TODO: insert other definitions and declarations here

//MOTOR X
static DigitalIoPin* STEPX;
static DigitalIoPin* DIRX;

//Limit Swtch X
static DigitalIoPin* LimitSWXMin;
static DigitalIoPin* LimitSWXMax;


//MOTOR Y
static DigitalIoPin* STEPY;
static DigitalIoPin* DIRY;

//Limit Swiches Y
static DigitalIoPin* LimitSWYMin;
static DigitalIoPin* LimitSWYMax;

static Motor*  MX;
static Motor*  MY;


static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

}



static void vTask1(void *pvParameters) {

	MX->calibration();
	MY->calibration();

	MX->calcStepCmRetio(xLength);
	MY->calcStepCmRetio(yLength);


}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	//Board_LED_Set(0, true);
#endif
#endif

	prvSetupHardware();
	// TODO: insert code here
	/* LED1 toggle thread */

	//MOTOR X
	STEPX = new DigitalIoPin(7,DigitalIoPin::output);
	DIRX = new DigitalIoPin(8,DigitalIoPin::output);

	//Limit Swtch X
	LimitSWXMin = new DigitalIoPin(3,DigitalIoPin::pullup,true);
	LimitSWXMax = new DigitalIoPin(4,DigitalIoPin::pullup,true);


	//MOTOR Y
	STEPY = new DigitalIoPin(5,DigitalIoPin::output);
	DIRY = new DigitalIoPin(6,DigitalIoPin::output);

	//Limit Swiches Y
	LimitSWYMin = new DigitalIoPin(0,DigitalIoPin::pullup,true);
	LimitSWYMax = new DigitalIoPin(1,DigitalIoPin::pullup,true);

	MX = new Motor(STEPX,DIRX, LimitSWXMin, LimitSWXMax);
	MY = new Motor(STEPY,DIRY, LimitSWYMin, LimitSWYMax);

	xTaskCreate(vTask1, "vTask1",configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),(TaskHandle_t *) NULL);
	//xTaskCreate(vTask2, "vTask2",configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),(TaskHandle_t *) NULL);
	/* Start the scheduler */
	vTaskStartScheduler();
	// Force the counter to be placed into memory
	// Enter an infinite loop, just incrementing a counter

	MX->stop();
	return 0 ;
}
