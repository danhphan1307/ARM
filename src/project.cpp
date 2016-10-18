
/*
 * @brief Blinky example using timers and sysTick
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "board_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DigitalIoPin.h"
#include <stdlib.h>
#include "../freertos/inc/semphr.h"
//#include "Macro.h"
#include "Syslog.h"
#include "Servo.h"
#include <string.h>
#include "Macros.h"
#include "Motor.h"
Syslog* syslog = new Syslog();

QueueHandle_t xQueue = xQueueCreate(50,sizeof(CommandStruct));


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

volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();


static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	Board_LED_Set(0, false);
}


static void calibrateTask(void *pvParameters) {
	MX->calibration();
	MY->calibration();

	MX->calcStepCmRetio(xLength);
	MY->calcStepCmRetio(yLength);
}

static void readCommand(void* param){
	Syslog* guard = (Syslog*)param;
	while(1){
		guard->getCommand(xQueue);

	}
}
static void readQueue(void* param){
	Servo pencil(0,10,160,90);
	CommandStruct commandToQueue;
	while(1){
		if (xQueueReceive(xQueue,&commandToQueue,( TickType_t ) 10)){
			if (commandToQueue.type ==SERVOR){
				pencil.Degree(commandToQueue.degreeServo);
			}
		}
	}
}

int main(void)
{
	prvSetupHardware();

	//Setup motor
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

	syslog->InitMap();
	xTaskCreate(readCommand, "readCommand",
			configMINIMAL_STACK_SIZE, syslog, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(readQueue, "readQueue",
			configMINIMAL_STACK_SIZE, syslog, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(calibrateTask, "calibrateTask",
			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	MX->stop();
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
