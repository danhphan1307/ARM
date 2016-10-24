/*
 * Utitlities.h
 *
 *  Created on: Oct 22, 2016
 *      Author: nguyenluong
 */

#ifndef UTITLITIES_H_
#define UTITLITIES_H_

namespace Utility{

int getDistance(int CurPosX, int CurPosY, int NewPosX, int NewPosY)
{
	return (sqrt( ((NewPosX - CurPosX)*(NewPosX - CurPosX)) + ((NewPosY - CurPosY)*(NewPosY - CurPosY)) ));
}


//lastvalue                newvalue
void BHL(Motor* MX, Motor* MY,int CurPosX, int CurPosY, int NewPosX, int NewPosY,void(*call)(int,int,RIT_TYPE,MotorType))
{
	int speed =MINSPEED;
	//int longerDelta = 0;

	int  OrigX, OrigY;
	//bool directionX, directionY;

	//calculate x y deltas
	int deltaX = abs(NewPosX-CurPosX);
	int deltaY = abs(NewPosY-CurPosY);

	//this decide direction
	int sx=1, sy=1;

	if(CurPosX<NewPosX){
		sx = 1;
		MX->setDir(false);
		//directionX=true;
	}else{
		sx = -1;
		MX->setDir(true);
		//directionX=false;
	}
	if(CurPosY<NewPosY){
		sy = 1;
		MY->setDir(false);
		//directionY=true;
	}else{
		sy = -1;
		MY->setDir(true);
		//directionY=false;
	}

	//at the beginning find out which delta is longer and put error in the middle of the line
	//if deltaX > deltaY error = deltaX else error = -deltaY
	//int error = (deltaX>deltaY ? deltaX : -deltaY)/2;
	//longerdelta for acceleration
	int error;

	if(deltaX > deltaY){
		error = deltaX/2;
		//longerDelta = deltaX;
	}else{
		error = -deltaY/2;
		//longerDelta = deltaY;
	}

	//old error variable
	int OldErr;

	//int rounds = 0;
	//int accelerationlenght= 0;
	char buffer[64];

	int origDis = getDistance(CurPosX, CurPosY, NewPosX, NewPosY);


	while(1){

		OrigX=CurPosX;
		OrigY=CurPosY;

		//break loop if last and new are same
		if (CurPosX==NewPosX && CurPosY==NewPosY)
		{
			speed = MINSPEED;
			break;
		}

		OldErr = error;
		//recalculate error
		//moves one or both. Depends on OldErr value. Longer delta moves always

		if(origDis > 3000)
		{
			if((origDis -getDistance(CurPosX, CurPosY, NewPosX, NewPosY)) >=0 && (origDis -getDistance(CurPosX, CurPosY, NewPosX, NewPosY)) < (origDis/4))
			{
				if((speed+40) < MAXSPEED)
				{
					speed +=40;
				}
				else
				{
					speed = MAXSPEED;
				}
			}
			else if((origDis -getDistance(CurPosX, CurPosY, NewPosX, NewPosY)) >= (3*origDis/4))
			{
				if((speed-40) > MINSPEED)
				{
					speed -=40;
				}
				else
				{
					speed = MINSPEED;
				}

			}
		}

		if (OldErr >-deltaX) {
			error -= deltaY;
			CurPosX += sx;
			//MX->move();
			call( 1, speed, RUN,X);
		}

		if (OldErr < deltaY) {
			error += deltaX;
			CurPosY += sy;
			call( 1, speed, RUN,Y);

		}
		//int disss = getDistance(CurPosX, CurPosY, NewPosX, NewPosY);
		//sprintf(buffer,"\r\nSpeed Was ( %d)\t Distance was: %d\r\n",speed, disss);
		//rounds++;
		//ITM_write(buffer);
	}
}
}





#endif /* UTITLITIES_H_ */
