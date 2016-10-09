/*
 * Stepper.h
 *
 *  Created on: Sep 23, 2016
 *      Author: nguyenluong
 */

#ifndef STEPPER_H_
#define STEPPER_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "Macro.h"
enum COMMAND_TYPE {LEFT, RIGHT, SPEED, NOT_A_COMMAND, GO, STOP};
class Stepper {
public:
	Stepper() {
		step = 0;
		dir =false;
		pps = 800;
		touchCount = 0;
		allowExecutionFlag = false; //default is off
		releaseSemaphoreFlag = false; //default is off
		calibrateFlag = true; //default is on: calibrate for first time sprinting
		type = NOT_A_COMMAND;
	}
	virtual ~Stepper() {}

	void run()
	{
		releaseSemaphoreFlag = false;
		PIN_DIR.write(dir);

		if (type == RIGHT){
			if(!SWITCH_RIGHT.read()){
				PIN_STEP.write(true);
				PIN_STEP.write(false);
			}
			else {
				PIN_STEP.write(false);
				releaseSemaphoreFlag = true;
			}
		}

		if (type == LEFT){
			if(!SWITCH_LEFT.read()){
				PIN_STEP.write(true);
				PIN_STEP.write(false);
			}
			else {
				PIN_STEP.write(false);
				releaseSemaphoreFlag = true;
			}
		}

	}


	//Calculate number of steps between 2 switches
	void calibrate(){
		//Check if switch is hit the second time
		if (touchCount==2){
			PIN_STEP.write(true);
			PIN_STEP.write(false);
			tempCountStep--;
			//Turn on flag to notify ISR that calibration is finished
			if (tempCountStep == countstep/2){
				calibrateFlag = false;
			}
		} else{
			//If motor hits switch (first time)
			if (touchCount==1){
				dir = status ? false : true; //revert direction (hit left sw -> run right, hit right sw, run lefT)
				PIN_DIR.write(dir);
				PIN_STEP.write(true);
				PIN_STEP.write(false);
				countstep++;
				//Check if motor hit switch for the second time
				if ( (status && SWITCH_RIGHT.read())
						|| (!status &&SWITCH_LEFT.read()) ){
					dir = !dir;
					PIN_DIR.write(dir);
					PIN_STEP.write(false);
					//Turn on second time hit flag
					tempCountStep = countstep; //store countstep into a temp for later evaluation
					touchCount++;
					return;
				}
			} else {
				//If motor has not hit switch, then just run
				if (!SWITCH_LEFT.read() && !SWITCH_RIGHT.read()){
					PIN_STEP.write(true);
					PIN_STEP.write(false);
				} else {
					//Turn off first time sprint if switch is hit
					PIN_STEP.write(false);
					status = SWITCH_LEFT.read();
					touchCount++;
				}
			}
		}
	}


	void receiveCommand(char* command, bool pending){
		//Board_UARTPutSTR(command);
		//Extract command words from string
		//Turn on flag execution if receive right or left movement command
		if (strstr(command,"left") != NULL){
			type = LEFT;
			allowExecutionFlag = true;
		} else if (strstr(command,"right") != NULL){
			type = RIGHT;
			allowExecutionFlag = true;
		} else if (strstr(command,"pps") != NULL){
			type = SPEED;
			allowExecutionFlag = false;
		} else if (strstr(command,"go") != NULL){
			type = GO;
			allowExecutionFlag = false;
		} else if (strstr(command,"stop") != NULL){
			type = STOP;
			allowExecutionFlag = false;
		} else {
			type = NOT_A_COMMAND;
			allowExecutionFlag = false;
		}
		Board_UARTPutSTR("\r\n");

		if (!pending){
			executeCommand(command,type);
		}

	}

	//Setup direction and speed for motor
	void executeCommand(char* command, COMMAND_TYPE type){
		long val;
		char  *p = command;
		while (*p) { // While there are more characters to process...
			if (isdigit(*p)) { // Upon finding a digit, ...
				val = strtol(p, &p, 10); // Read a number, ..
			} else { // Otherwise, move on to the next character.
				p++;
			}
		}

		char str[150];
		switch (type){
		case LEFT:
			dir = true;
			step = val;
			sprintf(str,"Running leftward with speed: %d\r\n",pps);
			Board_UARTPutSTR(str);
			break;
		case RIGHT:
			dir = false;
			step = val;
			sprintf(str,"Running rightward with speed: %d\r\n",pps);
			Board_UARTPutSTR(str);
			break;
		case SPEED:
			pps = val;
			step = 0;
			sprintf(str,"Speed set to: %d pps\r\n",pps);
			Board_UARTPutSTR(str);
			break;
		case GO:
			Board_UARTPutSTR("Begin execution\r\n");
			break;
		case STOP:
			Board_UARTPutSTR("Stop execution\r\n");
			break;
		case NOT_A_COMMAND:
			Board_UARTPutSTR("Not a valid command\r\n");
			break;
		}
	}

	int getStep() { return step;}
	int getPps() { return pps; }
	int getCountstep() { return countstep;}
	void resetCountstep() { countstep = 0;}

	bool getReleaseFlag(){ return releaseSemaphoreFlag; }
	bool getCalibrateFlag(){ return calibrateFlag;}
	COMMAND_TYPE getCommandType(){ return type; }
private:
	//Flag to signal that semaphore can be released
	bool releaseSemaphoreFlag;
	//Flag to allow execution , running motor in this case
	bool allowExecutionFlag;
	//Flag to signal that motor is in calibrating stage
	bool calibrateFlag;
	int step;
	bool dir; //direction
	int pps;
	int countstep; //step between 2 switches
	COMMAND_TYPE type; //type of command stepper needs to execute

	int touchCount;
	//Var for calibrating
	bool status;
	int tempCountStep;



};

#endif /* STEPPER_H_ */
