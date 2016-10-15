/*
 * Syslog.h
 *
 *  Created on: Sep 19, 2016
 *      Author: nguyenluong
 */

#ifndef SYSLOG_H_
#define SYSLOG_H_

#include <map>
#include <stdio.h>
#include <string.h>
#include <sstream>
using namespace std;
/* Sets up system hardware */
class Syslog {
public:
	Syslog();
	virtual ~Syslog();
	void write(char *description);
	void writeChar(char charNum);
	void writeInt(int number);
	void writeFloat(float number);
	void getCommand(QueueHandle_t xQueue);
	void InitMap();
private:
	SemaphoreHandle_t syslogMutex;
	int word_count = 0;
	char command[100] ;
	map<string, int> my_map;
};

Syslog::Syslog() {
	syslogMutex = xSemaphoreCreateMutex();
}
Syslog::~Syslog() {
	vSemaphoreDelete(syslogMutex);
}

void Syslog::write(char *description) {
	if(xSemaphoreTake(syslogMutex, DLY20MS) == pdTRUE) {
		Board_UARTPutSTR(description);
		xSemaphoreGive(syslogMutex);
	}
}

void Syslog::writeChar(char charNum) {
	if(xSemaphoreTake(syslogMutex, DLY20MS) == pdTRUE) {
		Board_UARTPutChar(charNum);
		xSemaphoreGive(syslogMutex);
	}
}

void Syslog::writeInt(int number){
	if(xSemaphoreTake(syslogMutex, DLY20MS) == pdTRUE) {
		char result[5];
		itoa(number,result,10);
		Board_UARTPutSTR(result);
		Board_UARTPutSTR("\r\n");
		xSemaphoreGive(syslogMutex);
	}
}
void Syslog::writeFloat(float number){
	if(xSemaphoreTake(syslogMutex, DLY20MS) == pdTRUE) {
		char result[5];
		sprintf(result, "%f", number);
		Board_UARTPutSTR(result);
		//Board_UARTPutSTR("\r\n");
		xSemaphoreGive(syslogMutex);
	}
}

void Syslog::InitMap(){
	my_map["M10"]  = 1;
	my_map["G28"]  = 2;
	my_map["M1 "]  = 3;
	my_map["M4 "]  = 4;
	my_map["G1 "]  = 5;
}

void Syslog::getCommand(QueueHandle_t xQueue){
	//This function will put the command to xQueue

	int num = Board_UARTGetChar();

	if (num!=EOF){
		command[word_count] = num;
		Board_UARTPutChar(num);
		word_count++;

		if(xSemaphoreTake(syslogMutex, DLY20MS) == pdTRUE) {

			if (num == 13 || num == 10 ){
				char cSplitCommand[4];
				strncpy(cSplitCommand, command, 3);
				cSplitCommand[3]='\0';
				string str(cSplitCommand);

				int t = my_map.find(str)->second;
				switch(t){
				case 1 :
					//M10
					//Go home and start drawing.

					break;
				case 2:
					//G28
					// Do something ?
					break;
				case 3:{
					//M1
					//for the pencil
					char  *p = command;
					int iDegree;
					while (*p) { // While there are more characters to process...
						if (isdigit(*p)) { // Upon finding a digit, ...
							iDegree = strtol(p, &p, 10); // Read a number, ..
						} else { // Otherwise, move on to the next character.
							p++;
						}
					}

					//degree will be load to iDegree. Use this for the pencil
					break;
				}

				case 4:
					//M4
					//laser
				{
					char  *p = command;
					int iDegree;
					while (*p) { // While there are more characters to process...
						if (isdigit(*p)) { // Upon finding a digit, ...
							iDegree = strtol(p, &p, 10); // Read a number, ..
						} else { // Otherwise, move on to the next character.
							p++;
						}
					}
					//degree will be load to iDegree. Use this for the laser
					break;
				}
				case 5:
					//G1
					//move the stepper according to X and Y
				{
					char* pch;
					pch = strtok (command," ");
					float geoX =0.0;
					float geoY =0.0;
					//split the command by space
					while (pch != NULL)
					{
						if(strncmp(pch,"A0",2)==0){
							//If end of the line, break;
							break;
						}else{
							if(strncmp(pch,"G1",2)==0){
								//G1 is the init of the command, donothing here
							}else{
								if(strncmp(pch,"X",1)==0){
									bool bIsFloat = false;
									while (*pch) { // While there are more characters to process...
										if (isdigit(*pch)) { // Upon finding a digit, ...
											if(bIsFloat){

												geoX += (float)strtol(pch, &pch, 10)/100;
											}else{
												geoX = strtol(pch, &pch, 10);
											}
										} else { // Otherwise, move on to the next character.
											if(strncmp(pch,".", 1)==0){
												bIsFloat = true;
											}
											pch++;
										}
									}
								}else if(strncmp(pch,"Y",1)==0){

									bool bIsFloat= false;
									while (*pch) { // While there are more characters to process...
										if (isdigit(*pch)) { // Upon finding a digit, ...
											if(bIsFloat){
												geoY += (float)strtol(pch, &pch, 10)/100;
											}else{
												geoY = strtol(pch, &pch, 10);
											}
										} else { // Otherwise, move on to the next character.
											if(strncmp(pch,".", 1)==0){
												bIsFloat = true;
											}
											pch++;
										}
									}
								}

							}
						}
						pch = strtok (NULL, " ");
					}
					geoX = roundf(geoX * 100) / 100;
					geoY = roundf(geoY * 100) / 100;
					break;
				}
				}

				//Send back OK, reset the word_count and reset the command
				Board_UARTPutSTR("\r\nOK\r\n");
				word_count=0;
				memset(command, 0, sizeof(command));
			}

			xSemaphoreGive(syslogMutex);
		}
	}
}

#endif /* SYSLOG_H_ */
