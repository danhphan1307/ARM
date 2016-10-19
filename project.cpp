
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
#include "Laser.h"
#include <string.h>
#include "Macros.h"
#include "Motor.h"

Syslog* syslog = new Syslog();
SemaphoreHandle_t calibrateSemaphore = xSemaphoreCreateBinary();
QueueHandle_t xQueue = xQueueCreate(50,sizeof(CommandStruct));
enum RIT_TYPE  {CALIBRATE, RUN}; // rit running type
RIT_TYPE RIT_type;
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

static Motor*  mInUse;

volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();


static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	Board_LED_Set(0, false);
	//Init RIT
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
}


extern "C" {
void RIT_IRQHandler(void)
{
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	switch (RIT_type){
	case CALIBRATE:
		if (MX->getCalibratedFlag()==false){
			MX->calibration();
		}
		else if (MY->getCalibratedFlag()==false){
			MY->calibration();
		}
		else {
			//distanceRatio = stepper.getCountstep()/10;
			Chip_RIT_Disable(LPC_RITIMER); // disable timer
			// Give semaphore and set context switch flag if a higher priority task was woken up
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
		}
		break;
	case RUN:
		mInUse->move();
		break;

	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

//The following function sets up RIT interrupt at given interval
//and waits until count RIT interrupts have occurred.
//Note that the actual counting is performed by the ISR and this function just waits on the semaphore.

void RIT_start(int count, int us, RIT_TYPE type, Motor *m)
{
	uint64_t cmp_value;
	mInUse = m;
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;

	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;

	RIT_type = type;
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	Chip_RIT_Enable(LPC_RITIMER);
	NVIC_EnableIRQ(RITIMER_IRQn);
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}


static void calibrateTask(void *pvParameters) {

	if (xSemaphoreTake(pvParameters,DLY20MS) == pdTRUE){

	}
	while(1){
		if (MX->getCalibratedFlag()==false){
			//A very large number of step to get it moving
			RIT_start(10000,TICK_RATE/MX->getpps(),CALIBRATE);
		}
	}
}

//This task read command from UART and put it to the Queue
static void readCommand(void* param){
	Syslog* guard = (Syslog*)param;
	while(1){
		if (xSemaphoreTake(calibrateSemaphore,(TickType_t) 10) == pdTRUE){
			guard->getCommand(xQueue);
		}
	}
}


int ppsCalc(int step, int xpos, int ypos )
{

	return (step/(sqrt((xpos*xpos) + (ypos*ypos) ))) * DefPPS;
}


/*This task wait for the semaphore, take it if it is free.
Then it will read command from the Queue and execute the command and release the semaphore.
 */

static void readQueue(void* param){
	Servo pencil(0,10);
	Laser laser(0,12);
	Syslog* guard = (Syslog*)param;
	CommandStruct commandToQueue;

	while(1){
		if (xQueueReceive(xQueue,&commandToQueue,( TickType_t ) 10)){
			if (commandToQueue.type ==SERVOR){
				pencil.Degree(commandToQueue.degreeServo);
				guard->write("OK\r\n");
			}else if (commandToQueue.type ==LASER){
				laser.Power(commandToQueue.power);
				guard->write("OK\r\n");
			}else if (commandToQueue.type ==BOTH_STEPPER){
				//commandToQueue.geoX
				//calculate the steps depend on the location
				//set the pps
				//call the pps depending on each motor step
				//afte the distance == 0 set the curnPos of the motor to the given one

				int ppsx = psCalc(MX->calculateMove(commandToQueue.geoX), MX->calculateMove(commandToQueue.geoX), MY->calculateMove(commandToQueue.geoY));
				int ppsy = psCalc(MY->calculateMove(commandToQueue.geoY), MX->calculateMove(commandToQueue.geoX), MY->calculateMove(commandToQueue.geoY));

				while(MX->calculateMove(commandToQueue.geoX)!=0 && MY->calculateMove(commandToQueue.geoY) !=0)
				{
					RIT_start(1,TICK_RATE/ppsx,RUN, MX);
					RIT_start(1,TICK_RATE/ppsy,RUN, MY);
				}
				MX->setCurPos(commandToQueue.geoX);
				MX->setCurPos(commandToQueue.geoY);



				guard->write("OK\r\n");
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
			configMINIMAL_STACK_SIZE, sbRIT, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	MX->stop();
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
