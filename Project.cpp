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

#include "board_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DigitalIoPin.h"
#include <stdlib.h>
#include "semphr.h"
#include "Syslog.h"
#include <string.h>
#include "Macros.h"
#include "Motor.h"

Syslog* syslog = new Syslog();
SemaphoreHandle_t calibrateSemaphore = xSemaphoreCreateBinary();
//QueueHandle_t xQueue = xQueueCreate(50,sizeof(CommandStruct));
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

Motor* mInUse;

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
		break;

	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

//The following function sets up RIT interrupt at given interval
//and waits until count RIT interrupts have occurred.
//Note that the actual counting is performed by the ISR and this function just waits on the semaphore.

void RIT_start(int count, int us, RIT_TYPE type)
{
	uint64_t cmp_value;

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
	bool calib = false;

	if (xSemaphoreTake(pvParameters,DLY1MS) == pdTRUE){

	}
	while(1){
		calib = (MX->getCalibratedFlag()&& MY->getCalibratedFlag() );
		while (!calib){
			//A very large number of step to get it moving
			RIT_start(1,TICK_RATE/MX->getpps(),CALIBRATE);
		}
	}
}

/*
static void readCommand(void* param){
	Syslog* guard = (Syslog*)param;
	while(1){
		if (xSemaphoreTake(calibrateSemaphore,(TickType_t) 10) == pdTRUE){
			guard->getCommand(xQueue);
		}
	}
}

static void readQueue(void* param){
	Servo pencil(0,10,160,90);
	CommandStruct commandToQueue;
	while(1){
		if (xSemaphoreTake(calibrateSemaphore,(TickType_t) 10) == pdTRUE){
			if (xQueueReceive(xQueue,&commandToQueue,( TickType_t ) 10)){
				if (commandToQueue.type ==SERVOR){
					pencil.Degree(commandToQueue.degreeServo);
				}
			}
		}
	}
}

*/

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

	//syslog->InitMap();
	//xTaskCreate(readCommand, "readCommand",configMINIMAL_STACK_SIZE, syslog, (tskIDLE_PRIORITY + 1UL),(TaskHandle_t *) NULL);
	//xTaskCreate(readQueue, "readQueue",configMINIMAL_STACK_SIZE, syslog, (tskIDLE_PRIORITY + 1UL),(TaskHandle_t *) NULL);

	xTaskCreate(calibrateTask, "calibrateTask",
			configMINIMAL_STACK_SIZE, sbRIT, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	//MX->stop();
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
