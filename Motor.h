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

#endif /* MOTOR_H_ */


enum cmdt{left, right, PPS, DY, not_available, calib, block, STOP};
//enum papirtype{left, right, PPS, DY, not_available, calib, block, STOP};

enum moveDirType{mLeft, mRight};




class Motor {
public:

	//done
	//Motor();
	Motor(DigitalIoPin* S, DigitalIoPin* D, DigitalIoPin* Lmin, DigitalIoPin* Lmax);
	virtual ~Motor();

	//Calibration functions

	void setStepNum(int s);
	int getStepNum();
	void resetStepNum();

	int getStepMin();
	void setStepMin(int s);

	int getStepMax();
	void setStepMax(int s);

	int getMarginMin();
	void setMarginMin(int m);

	int getMarginMax();
	void setMarginMax(int m);

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

	/*void executecmd(cmdque cmds);
	bool getReleseFlag();
	void setReleseFlag(bool b);
	 */


	float getCurPos();
	void setCurPos(float p);

	bool getAllowFlag();
	void setAllowFlag(bool b);

	bool getRefRITT();
	void setRefRITT(bool b);

	//DOne
	void setMoveType(moveDirType t);
	moveDirType getMoveType();

private:
	int touchCount;
	int stepCount;

	//bool dir;

	int Maxstepnum;

	bool allow;

	bool calibrated;

	bool release;

	int pps;

	bool refresh;

	moveDirType moveType;

	float CurPos;

	int stepMin;
	int stepMax;


	float minStepCmMRetio;
	float maxStepCmMRetio;

	int marginMin;
	int marginMax;

	DigitalIoPin* STEP;
	DigitalIoPin* DIR;

	DigitalIoPin* LimitSWMin;
	DigitalIoPin* LimitSWMax;

	bool pulse = true;
	//Flag to signal that semaphore can be released
	bool releaseSemaphoreFlag;
	//Flag to allow execution , running motor in this case
	bool allowExecutionFlag;
	//Flag to signal that motor is in calibrating stage
	bool calibrateFlag;
	int step;
	bool dir; //direction
	int countstep; //step between 2 switches
	//Var for calibrating
	bool status;
	int tempCountStep;
};
