
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
#include "Macro.h"
#include "Stepper.h"
#include "Syslog.h"
#include <string.h>

Syslog* syslog = new Syslog();
Stepper stepper;

#define TICK_RATE 1000000
volatile uint32_t RIT_count;
volatile bool RIT_calibrate;
enum RIT_TYPE  {CALIBRATE, COMMAND};
RIT_TYPE RIT_type;
xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();
QueueHandle_t xQueue = xQueueCreate(5,sizeof(char[100]));
static xSemaphoreHandle semaphore = xSemaphoreCreateBinary();
volatile bool stopFlag = false;


static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	//Init RIT
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
}


//RIT Handler
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
		if (stepper.getCalibrateFlag()){
			stepper.calibrate();
		} else {
			Chip_RIT_Disable(LPC_RITIMER); // disable timer
			// Give semaphore and set context switch flag if a higher priority task was woken up
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
		}
		break;
	case COMMAND:
		if(RIT_count > 0) {
			RIT_count--;
			// do something useful here...
			stepper.run();
			//Give back semaphore when switch is on
			if (stepper.getReleaseFlag()){
				RIT_count = 0;
			}

		}
		else {
			Chip_RIT_Disable(LPC_RITIMER); // disable timer
			// Give semaphore and set context switch flag if a higher priority task was woken up
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
		}
		break;
	}

	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}


void RIT_start(int count, int us, RIT_TYPE type)
{
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	RIT_type = type;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);

	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}

//UART Semaphore task
int word_count =0;
char command[100] = "";
bool stop = true;
//Task for UART Input
static void UARTTask(void *params) {
	while (1) {

		if (stepper.getCalibrateFlag()){
			//A very large number of step to get it moving
			RIT_start(10000,TICK_RATE/stepper.getPps(),CALIBRATE);
		} else {
			int num = Board_UARTGetChar();
			if (num!=EOF){
				command[word_count] = num;
				Board_UARTPutChar(num);
				word_count++;
				if (num == 13 || num == 10 ){
					if (strstr(command,"go") != NULL){
						//xSemaphoreGive(params);
						stop = false;
					} else if (strstr(command,"stop") != NULL){
						if (xQueueSendToFront(xQueue,command,(TickType_t) 10)==pdTRUE){}
					} else {
						if (xQueueSendToBack(xQueue,command,(TickType_t) 10) == pdTRUE){

						} else {
							Board_UARTPutSTR("Queue is full\r\n");
						}
					}
					Board_UARTPutSTR("\r\n");
					word_count=0;
					memset(command, 0, sizeof(command));
				}
			}
		}
	}
}

static void CommandTask(void *params) {

	while(1){

		while (!stop){
			char receive[100];
			while (xQueueReceive(xQueue,receive, (TickType_t) 10) && !stop){
				if (strstr(receive,"stop") != NULL){
					stop = true;
				} else {
					stepper.receiveCommand(receive,false);
					RIT_start(stepper.getStep(),TICK_RATE/stepper.getPps(),COMMAND);
				}
			}
		}
	}

}


void showCommandGuideline(){
	Board_UARTPutSTR("Command guideline: \r\n");
	Board_UARTPutSTR("left <count>: runs stepper to <count> steps leftward \r\n");
	Board_UARTPutSTR("right <count>: runs stepper to <count> steps rightward \r\n");
	Board_UARTPutSTR("pps <count>: set number of driving pulses per sec \r\n");
}


int main(void)
{
	prvSetupHardware();

	xTaskCreate(UARTTask, "UARTTask",
			configMINIMAL_STACK_SIZE, semaphore, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(CommandTask, "CommandTask",
			configMINIMAL_STACK_SIZE, semaphore, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	showCommandGuideline();

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
