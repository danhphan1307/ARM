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

	//End calibration functions

	//Movment Functions


	void setDir(bool d);
	bool getDir();


	//void move(int newPos);
	void move();
	int calculateMove(int newPos);


	void stop();
	void reverse();


	int getpps();
	void setpps(int b);
	//end movment functions

	/*void executecmd(cmdque cmds);
	bool getReleseFlag();
	void setReleseFlag(bool b);
	 */


	bool getAllowFlag();
	void setAllowFlag(bool b);

	bool getRefRITT();
	void setRefRITT(bool b);

	//DOne
	void setMoveType(moveDirType t);
	moveDirType getMoveType();

private:
	int stepCount;

	//bool dir;

	int Maxstepnum;

	bool allow;

	bool calibrated;

	bool release;

	int pps;

	bool refresh;

	moveDirType moveType;

	int CurPos;

	int stepMin;
	int stepMax;

	int marginMin;
	int marginMax;

	DigitalIoPin* STEP;
	DigitalIoPin* DIR;

	DigitalIoPin* LimitSWMin;
	DigitalIoPin* LimitSWMax;


};


