/*
 * Motor.h
 *
 *  Created on: 9.10.2016
 *      Author: abdullai
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "Macros.h"
#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
#endif /* MOTOR_H_ */

enum cmdt{left, right, PPS, DY, not_available, calib, block, STOP};

enum MotorType{X,Y};

class Motor {
public:

	//Motor();
	Motor(DigitalIoPin* S, DigitalIoPin* D, DigitalIoPin* Lmin, DigitalIoPin* Lmax,MotorType t,void(*call)(int,int,RIT_TYPE,MotorType));
	virtual ~Motor();

	//Calibration functions

	void calibration();
	bool getCalibratedFlag();
	void setCalibratedFlag(bool c);

	//End calibration functions

	//Movment Functions


	void setDir(bool d);
	bool getDir();

	void setAllow(bool d);
	bool getAllow();


	//void move(int newPos);
	void move();
	int calculateMove(float newPos);

	void calcStepCmRetio(int cm);


	void stop();
	void reverse();

	int getpps();
	void setpps(int b);
	//end movment functions

	float getCurPos() {return curPos;}
	void setCurPos(float p) {curPos = p;}

	bool getAllowFlag();
	void setAllowFlag(bool b);

	bool getRefRITT();
	void setRefRITT(bool b);


	//Move by geo
	float getCountStepToMmRatio() { return mmToStepRatio;}
	bool isHit() {return LimitSWMax->read()||LimitSWMin->read();}
	//Move by geo
	void move(float geo);
	void swichpin();

	//Stepping function
	void stepUp() {if (xSemaphoreTake(stepSemaphore,(TickType_t)1)){STEP->write(true);}}
	void stepDown() {if (xSemaphoreTake(stepSemaphore,(TickType_t)1)){STEP->write(false);}}

	//Accelerating and decelerating
	void speeding(int steps);

	void giveSemaphore(){ xSemaphoreGive(stepSemaphore);}
private:
	float lengthInMm;
	int touchCount;
	int stepCount;
	int margin;
	SemaphoreHandle_t stepSemaphore;
	int margin;
	//bool dir;

	float curPos;
	bool release;

	int pps;
	float errorRatio; //acceptable error ratio when calibrating


	DigitalIoPin* STEP;
	DigitalIoPin* DIR;

	DigitalIoPin* LimitSWMin;
	DigitalIoPin* LimitSWMax;

	float mmToStepRatio;


	bool calibrated;
	int step;
	bool dir; //direction



	//Var for calibrating
	bool status;
	int tempstep;
	int tempstep;
	MotorType type;
	int touchCount;
	int stepCount;

	//Callback for RIT
	void (*call)(int,int,RIT_TYPE,MotorType);
};
