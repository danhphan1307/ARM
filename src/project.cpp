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
//#include "Utitlities.h"

//using namespace Utility;
Syslog* syslog = new Syslog();
SemaphoreHandle_t calibrateSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t runningSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t countingRun = xSemaphoreCreateCounting(20,0);
QueueHandle_t xQueue = xQueueCreate(50,sizeof(CommandStruct));

RIT_TYPE RIT_type;
Motor* mInUse;
volatile int runningMotor = 0;
volatile int counterRatio = 0;
bool doneRunning = false;

struct StepCount{
	int x;
	int y;
	StepCount(int _x, int _y){
		x =_x;
		y=_y;
	}
};
volatile StepCount countStruct(0,0);
volatile StepCount dataCountStruct(0,0);
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
	//Init RIT
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
}

static void calibrate(){
	while (MX->getCalibratedFlag()==false){
		MX->calibration();
	}
	while (MY->getCalibratedFlag()==false){
		MY->calibration();
	}
}

extern "C" {
void RIT_IRQHandler(void)
{
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag

	if (RIT_count > 0) {

		mInUse->swichpin();
		RIT_count--;
	}
	else {

		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

//The following function sets up RIT interrupt at given interval
//and waits until count RIT interrupts have occurred.
//Note that the actual counting is performed by the ISR and this function just waits on the semaphore.
static void RIT_start(int count, int us, RIT_TYPE type,MotorType _runningMotor)
{
	uint64_t cmp_value;
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) 1/us;
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	RIT_type = type;

	if(_runningMotor == X)
	{
		mInUse=MX;
	}
	if(_runningMotor == Y)
	{
		mInUse=MY;
	}

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


int getDistance(int CurPosX, int CurPosY, int NewPosX, int NewPosY)
{
	return (sqrt( ((NewPosX - CurPosX)*(NewPosX - CurPosX)) + ((NewPosY - CurPosY)*(NewPosY - CurPosY)) ));
}


//lastvalue                newvalue
void BHL(int CurPosX, int CurPosY, int NewPosX, int NewPosY)
{
	int speed =MINSPEED;


	int  OrigX, OrigY;


	//calculate x y deltas
	int deltaX = abs(NewPosX-CurPosX);
	int deltaY = abs(NewPosY-CurPosY);

	//this decide direction
	int sx=1, sy=1;

	if(CurPosX<NewPosX){
		sx = 1;
		MX->setDir(false);

	}else{
		sx = -1;
		MX->setDir(true);

	}
	if(CurPosY<NewPosY){
		sy = 1;
		MY->setDir(false);
		//directionY=true;
	}else{
		sy = -1;
		MY->setDir(true);
		//directionY=false;
	}

	//at the beginning find out which delta is longer and put error in the middle of the line
	//if deltaX > deltaY error = deltaX else error = -deltaY
	//int error = (deltaX>deltaY ? deltaX : -deltaY)/2;
	//longerdelta for acceleration
	int error;

	if(deltaX > deltaY){
		error = deltaX/2;
		//longerDelta = deltaX;
	}else{
		error = -deltaY/2;
		//longerDelta = deltaY;
	}

	//old error variable
	int OldErr;

	//int rounds = 0;
	//int accelerationlenght= 0;
	char buffer[64];

	int origDis = getDistance(CurPosX, CurPosY, NewPosX, NewPosY);


	while(1){

		OrigX=CurPosX;
		OrigY=CurPosY;

		//break loop if last and new are same
		if (CurPosX==NewPosX && CurPosY==NewPosY)
		{
			speed = MINSPEED;
			break;
		}

		OldErr = error;
		//recalculate error
		//moves one or both. Depends on OldErr value. Longer delta moves always

		if(origDis > 3000)
		{
			if((origDis -getDistance(CurPosX, CurPosY, NewPosX, NewPosY)) >=0 && (origDis -getDistance(CurPosX, CurPosY, NewPosX, NewPosY)) < (origDis/4))
			{
				if((speed+40) < MAXSPEED)
				{
					speed +=40;
				}
				else
				{
					speed = MAXSPEED;
				}
			}
			else if((origDis -getDistance(CurPosX, CurPosY, NewPosX, NewPosY)) >= (3*origDis/4))
			{
				if((speed-40) > MINSPEED)
				{
					speed -=40;
				}
				else
				{
					speed = MINSPEED;
				}

			}
		}

		if (OldErr >-deltaX) {
			error -= deltaY;
			CurPosX += sx;
			RIT_start( 1, speed, RUN,X);
		}

		if (OldErr < deltaY) {
			error += deltaX;
			CurPosY += sy;
			RIT_start( 1, speed, RUN,Y);

		}
		//int disss = getDistance(CurPosX, CurPosY, NewPosX, NewPosY);
		//sprintf(buffer,"\r\nSpeed Was ( %d)\t Distance was: %d\r\n",speed, disss);
		//rounds++;

	}

}


void moveMotor(CommandStruct commandToQueue){
	int oldXInMm = int(MX->getCurPos()*MX->getCountStepToMmRatio());
	int oldYInMm = int(MY->getCurPos()*MY->getCountStepToMmRatio());
	int newXInMm = int(commandToQueue.geoX*MX->getCountStepToMmRatio());
	int newYInMm = int(commandToQueue.geoY*MY->getCountStepToMmRatio());
	BHL(oldXInMm, oldYInMm, newXInMm, newYInMm);
	//BHL(MX->getCurPos()*43.5, MY->getCurPos()*43.5, commandToQueue.geoX*43.5, commandToQueue.geoY*43.5);
	MX->setCurPos(commandToQueue.geoX);
	MY->setCurPos(commandToQueue.geoY);
}
/*
 *This task read command from UART and put it to the Queue
 */
static void readCommand(void* param){
	calibrate();
	Syslog* guard = (Syslog*)param;
	while(1){
		if ( xQueue != 0){
			guard->getCommand(xQueue);
			//
		}
	}
}



/*
 * This task will check if there is anything in the queue and then execute it.
 */
static void readQueue(void* param){

	Servo pencil(0,10);
	Laser laser(0,12);
	Syslog* guard = (Syslog*)param;
	CommandStruct commandToQueue;


	while(1){
		if ( xQueue != 0  && xQueueReceive(xQueue,&commandToQueue,( TickType_t ) 10)){
			if (commandToQueue.type ==SERVOR){
				pencil.Degree(commandToQueue.degreeServo);
				guard->write("OK\r\n");
			}else if (commandToQueue.type ==LASER){
				laser.Power(commandToQueue.power);
				guard->write("OK\r\n");
			}else if (commandToQueue.type ==BOTH_STEPPER){
				//Motor code goes here.
				moveMotor(commandToQueue);
				guard->write("OK\r\n");
			}
		}
	}

}





int main(void)
{
	prvSetupHardware();
	/* Set up motor*/
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
	MX = new Motor(STEPX,DIRX, LimitSWXMin, LimitSWXMax,X,&RIT_start);
	MY = new Motor(STEPY,DIRY, LimitSWYMin, LimitSWYMax,Y,&RIT_start);
	/* End of set up motor*/
	syslog->InitMap();



	xTaskCreate(readCommand, "readCommand",
			configMINIMAL_STACK_SIZE, syslog, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(readQueue, "readQueue",
			configMINIMAL_STACK_SIZE, syslog, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/*
	xTaskCreate(calibrateTask, "calibrateTask",
			configMINIMAL_STACK_SIZE, calibrateSemaphore, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	 /*
	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
