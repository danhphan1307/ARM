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
	stage=0;
	errorRatio = 0.005;

	//Calibration
	tempstep = 0;
	margin = 500;
	stepCount = 0;
	step=0;
	type=t;

	lengthInMm = type == X ? 355 : 310;
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
	//Check if switch is hit to stop motor
	if (isHit()){
		stop();
	}
	//Else, call RIT function to run motor
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
	//Optinonal stage, used to move motor to home position
	if (stage==4){
		move();
		tempstep++;
		if(tempstep==step){
			reverse();
			calibrated = true; //turn off calibrate
			mmToStepRatio = (float)step / lengthInMm; //calculate ratio
		}
	}
	//Calculate step again, store in tempstep variable
	//If (tempstep-step) is larger than acceptable error prone, re-calibrate
	else if (stage==3){
		move();
		tempstep++;
		if (tempstep==step){

			step-=margin;
			if (type==X){
				//Check if X motor is at home
				if (!status){
					reverse();
					calibrated = true; //turn off calibration
					mmToStepRatio = (float)step / lengthInMm; //calculate ratio
					return;
				}
				//if not, reverse motor and run back to home
				else {
					reverse();
					tempstep=0;
					stage++;
				}
			} else {
				//save as above for Y motor
				if (!status){
					reverse();
					calibrated = true; //turn off calibration
					mmToStepRatio = (float)step / lengthInMm; //calculate ratio
					return;
				}
				else {
					reverse();
					tempstep=0;
					stage++;
				}
			}
		}
	}
	else if (stage==2){

		move();
		tempstep++;
		if ( (status && LimitSWMin->read())
				|| (!status &&LimitSWMax->read()) ){
			reverse();
			//Update status
			status=LimitSWMax->read();
			//Recalculate if exceeds error ratio
			if (abs((tempstep-step))>step*errorRatio){
				stage = 0;
				//recalculate
				tempstep=0;
				step=0;
				return;
			}
			//else, subtract margin to prevent switch hit
			//and move onto next stage of calibration
			else {

				step-=margin;
				tempstep=0;
				stage++;
			}

		}

	} else if (stage==1){
		move();
		step++;
		//Check if motor hit switch for the second time
		if ( (status && LimitSWMin->read())
				|| (!status &&LimitSWMax->read()) ){
			reverse();
			stop();
			//Turn on second time hit flag
			stage++;
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
			stage++;
		}
	}
}


