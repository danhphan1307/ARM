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
	pps = 500;

	minStepCmMRetio = 0.0;
	maxStepCmMRetio = 0.0;
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

	while(!calibrated) {

		if(hitCount >3 && !calibrated)
		{

			Board_UARTPutSTR("\r\nI reached the end !!!\r\n");
			sprintf(ch,"Margin Min: %d,\tMargin Max: %d,\tStep Min: %d,\t Step Max: %d\r\n\r\n", marginMin, marginMax, stepMin, stepMax);
			Board_UARTPutSTR(ch);

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
				//vTaskDelay(DLY5SEC);
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
				//vTaskDelay(DLY5SEC);
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

