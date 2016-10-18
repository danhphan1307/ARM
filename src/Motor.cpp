/*
 * Motor.cpp
 *
 *  Created on: 10.10.2016
 *      Author: abdullai
 */
#include "Motor.h"
#include <math.h>


Motor::Motor(DigitalIoPin* S, DigitalIoPin* D, DigitalIoPin* Lmin, DigitalIoPin* Lmax)
{
	calibrated = false;
	STEP = S;
	DIR = D;

	LimitSWMin = Lmin;
	LimitSWMax = Lmax;
	STEP->write(false);

	stepCount = 0;
	Maxstepnum = 0;
	CurPos = 0;
	pps = 3000;

	minStepCmMRetio = 0.0;
	maxStepCmMRetio = 0.0;
	DIR->write(false);

	 touchCount=0;

}




Motor::~Motor()
{
}

//Movment functions

void Motor::stop()
{
	STEP->write(false);


}


void Motor::reverse()
{
	DIR->write(!DIR->read());

}


//Step counting and calulations functions
/*
 void Motor::move(int newPos)
{
}
 */
void Motor::move()
{
	STEP->write(true);
	stepCount++;
	STEP->write(false);
}


void Motor::setStepNum(int s)
{
	stepCount = s;  
}

int Motor::getStepNum()
{
	return stepCount;  
}

void Motor::resetStepNum()
{
	stepCount = 0;
}


void Motor::setStepMin(int s)
{
	stepMin = s;
}

int Motor::getStepMin()
{
	return stepMin;
}

void Motor::setStepMax(int s)
{
	stepMax = s;
}

int Motor::getStepMax()
{
	return stepMax;
}



void Motor::setMarginMin(int m)
{
	marginMin = m;
}

int Motor::getMarginMin()
{
	return marginMin;
}

void Motor::setMarginMax(int m)
{
	marginMax = m;
}

int Motor::getMarginMax()
{
	return marginMax;
}


//DIrections functions
void Motor::setDir(bool d)
{
	DIR->write(d);
}

bool Motor::getDir()
{
	return DIR->read();
}


// Gives the motor a pps values for the stepping / half stepping 
int Motor::getpps()
{
	return pps;
}

void Motor::setpps(int b)
{
	pps = b;
}


void Motor::setMoveType(moveDirType t)
{
	moveType = t;
}

moveDirType Motor::getMoveType()
{
	return moveType;
}


float Motor::getCurPos()
{
	return CurPos;
}
void Motor::setCurPos(float p)
{
	CurPos = p;
}


//Calibrate



bool Motor::getCalibratedFlag()
{
	return calibrated;
}
void Motor::setCalibratedFlag(bool c)
{
	calibrated = c;
}


void  Motor::calcStepCmRetio(int um)
{
	minStepCmMRetio = stepMin/um;
	maxStepCmMRetio = stepMax/um;
}

int Motor::calculateMove(float newPos)
{
	if(CurPos > newPos)
	{
		DIR->write(true);

		return round(((CurPos - newPos) * minStepCmMRetio));

	}
	else if(CurPos < newPos)
	{
		DIR->write(false);
		return round(((newPos - CurPos) * maxStepCmMRetio));
	}
	else
	{
		STEP->write(false);
	}
	return 0;
}



void Motor::calibration() {
	int hitCount = 0;

	char ch[200] = {0};
	if (touchCount==2){
		move();
		tempCountStep--;

		//Turn on flag to notify ISR that calibration is finished
		if ( (status && LimitSWMin->read())|| (!status &&LimitSWMax->read()) )
		{
			dir = !dir;
			DIR->write(dir);
			STEP->write(false);

			//Turn on second time hit flag
			//Return if 2nd run error offset is ok ( +-3 steps offset )
			if (tempCountStep < 3 && tempCountStep > -3){
				calibrated = true;
			} else {
				//Reset calibration if condition is not met
				countstep =0;
				touchCount=0;
			}
		}
	} else{
		//If motor hits switch (first time)
		if (touchCount==1){
			dir = status ? false : true; //revert direction (hit left sw -> run right, hit right sw, run lefT)
			DIR->write(dir);
			move();
			countstep++;
			//Check if motor hit switch for the second time
			if ( (status && LimitSWMin->read())
					|| (!status &&LimitSWMax->read()) ){
				dir = !dir;
				DIR->write(dir);
				stop();
				//Turn on second time hit flag
				tempCountStep = countstep; //store countstep into a temp for later evaluation
				touchCount++;
				status=LimitSWMax->read();
				return;
			}
		} else {
			//If motor has not hit switch, then just run
			if (!LimitSWMax->read() && !LimitSWMin->read()){
				move();
			} else {
				//Turn off first time sprint if switch is hit
				stop();
				status = LimitSWMax->read();
				touchCount++;
			}
		}
	}
}
