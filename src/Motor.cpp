/*
 * Motor.cpp
 *
 *  Created on: 10.10.2016
 *      Author: abdullai
 */
#include "Motor.h"





Motor::Motor(DigitalIoPin* S, DigitalIoPin* D,
		DigitalIoPin* Lmin, DigitalIoPin* Lmax,MotorType t,
		void(*_call)(int,int,RIT_TYPE,MotorType))
{
	calibrated = false;
	STEP = S;
	DIR = D;
	LimitSWMin = Lmin;
	LimitSWMax = Lmax;
	STEP->write(false);
	DIR->write(false);

	pps = 30000;
	touchCount=0;
	errorRatio = 0.005;

	//Calibration
	tempstep = 0;
	margin = 500;
	stepCount = 0;
	step=0;
	type=t;

	lengthInMm = type == X ? 347 : 310;
	call = _call;

	curPos = 0.0;
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
void Motor::move(int _pps = 40000)
{
	call( 1, _pps, RUN,type);
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

void Motor::swichpin()
{
	STEP->write(!STEP->read());
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


void Motor::calibration() {
	if (touchCount==4){
		move();
		tempstep++;
		if(tempstep==step){
			reverse();
			calibrated = true; //turn off calibrate
			mmToStepRatio = (float)step / lengthInMm; //calculate ratio
		}
	}
	else if (touchCount==3){
		move();
		tempstep++;
		if (tempstep==step){

			step-=margin;
			if (type==X){
				if (!status){
					reverse();
					calibrated = true; //turn off calibrate
					mmToStepRatio = (float)step / lengthInMm; //calculate ratio
					return;
				}
				else {
					reverse();
					tempstep=0;
					touchCount++;
				}
			} else {
				if (!status){
					reverse();
					calibrated = true; //turn off calibrate
					mmToStepRatio = (float)step / lengthInMm; //calculate ratio
					return;
				}
				else {
					reverse();
					tempstep=0;
					touchCount++;
				}
			}
		}
	}
	else if (touchCount==2){

		move();
		tempstep++;
		if ( (status && LimitSWMin->read())
				|| (!status &&LimitSWMax->read()) ){
			reverse();

			status=LimitSWMax->read();

			/*step-=margin;
			tempstep=0;
			touchCount++; //increase touch count
			*/
			if (abs((tempstep-step))>step*errorRatio){
				touchCount = 0;
				//recalculate
				tempstep=0;
				step=0;
				return;
			}
			else {
				step-=margin;
				tempstep=0;
				touchCount++; //increase touch count
			}

		}

	} else if (touchCount==1){
		//If motor hits switch (first time)
		move();
		step++;
		//Check if motor hit switch for the second time
		if ( (status && LimitSWMin->read())
				|| (!status &&LimitSWMax->read()) ){
			reverse();
			stop();
			//Turn on second time hit flag
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

			status = LimitSWMax->read();
			DIR->write(!status);
			stop();
			touchCount++;
		}
	}
}


