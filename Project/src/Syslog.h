/*
 * Syslog.h
 *
 *  Created on: Sep 19, 2016
 *      Author: nguyenluong
 */

#ifndef SYSLOG_H_
#define SYSLOG_H_

#include <stdio.h>
#include <string.h>
#include <sstream>
/* Sets up system hardware */
class Syslog {
public:
	Syslog();
	virtual ~Syslog();
	void write(char *description);
	void writeChar(char charNum);
	void writeInt(int number);
	void getCommand();
private:
	SemaphoreHandle_t syslogMutex;
	int word_count = 0;
	char command[100] ;
};

Syslog::Syslog() {
	syslogMutex = xSemaphoreCreateMutex();
}
Syslog::~Syslog() {
	vSemaphoreDelete(syslogMutex);
}

void Syslog::write(char *description) {
	if(xSemaphoreTake(syslogMutex, LONG_DELAY) == pdTRUE) {
		Board_UARTPutSTR(description);
		xSemaphoreGive(syslogMutex);
	}
}


void Syslog::writeChar(char charNum) {
	if(xSemaphoreTake(syslogMutex, LONG_DELAY) == pdTRUE) {
		Board_UARTPutChar(charNum);
		xSemaphoreGive(syslogMutex);
	}
}

void Syslog::writeInt(int number){
	if(xSemaphoreTake(syslogMutex, LONG_DELAY) == pdTRUE) {
		char result[5];
		itoa(number,result,10);
		Board_UARTPutSTR(result);
		Board_UARTPutSTR("\r\n");
		xSemaphoreGive(syslogMutex);
	}
}


void Syslog::getCommand(){

	int num = Board_UARTGetChar();

	if (num!=EOF){

		command[word_count] = num;
		Board_UARTPutChar(num);
		word_count++;
		if(xSemaphoreTake(syslogMutex, LONG_DELAY) == pdTRUE) {
			if (num == 13 || num == 10 ){
				Board_UARTPutSTR("\r\n");
				Board_UARTPutSTR(command);
				word_count=0;
				memset(command, 0, sizeof(command));
			}
			xSemaphoreGive(syslogMutex);
		}
	}
}

#endif /* SYSLOG_H_ */
