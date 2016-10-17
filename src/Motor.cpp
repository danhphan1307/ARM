/*
 * Motor.cpp
 *
 *  Created on: 10.10.2016
 *      Author: abdullai
 */
#include "Motor.h"

//

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
	pps = 0;
	DIR->write(false);

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
	vTaskDelay(DLY1MS);
	STEP->write(false);
	stepCount++;
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

void Motor::calibration() {
	int hitCount = 0;

	while(!calibrated) {

		if(hitCount >3 && !calibrated)
		{

			Board_UARTPutSTR("\r\nI reached the end !!! ");

			switch(moveType)
			{
			case mLeft:
				while(stepCount < 100*marginMax)
				{
					move();
				}
				break;
			case mRight:
				while(stepCount < 100*marginMin)
				{
					move();
				}
				break;
			}

			calibrated = true;
			stop();

		}
		else if((!LimitSWMin->read() && !LimitSWMax->read()) && !calibrated) {

			move();
		}
		else if(!calibrated)
		{

			reverse();
			hitCount++;
			if(LimitSWMin->read())
			{
				moveType = mLeft;

				Board_UARTPutSTR("\r\nLimit Min Hit");
				if(hitCount >1)
				{
					stepMin= stepCount;
				}

				resetStepNum();
				while(LimitSWMin->read())
				{
					move();
				}
				marginMin= stepCount;
			}
			else if(LimitSWMax->read())
			{
				moveType = mRight;
				Board_UARTPutSTR("\r\nLimit Max Hit");
				if(hitCount >1)
				{
					stepMax= stepCount;
				}
				resetStepNum();
				while(LimitSWMax->read())
				{
					move();
				}
				marginMax= stepCount;
			}
			resetStepNum();




		}

		stop();


	}

}

