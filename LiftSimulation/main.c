/*
 * GccApplication1.c
 *
 * Created: 26.11.2019 16:29:04
 * Author : rolfl
 */ 

#include <avr/io.h>

//#include "LiftLibrary.h"
#include "AppIncludes.h"

void TestFunction(uint8_t* data, uint8_t nrOfBytes);

int main(void)
{
    InitializePorts();
	Usart_Init();
	RegisterTestHandler( TestFunction );
	RegisterFsm(&_motorCtrl.fsm);
	RegisterFsm(&_mainCtrl.fsm);
	// letzte Funktion die aufgerufen wird!
	InitializeStart();
}

void TestFunction(uint8_t* data, uint8_t nrOfBytes)
{
    if( data[0] < 4 )
	{
		SendEvent(SignalSourceApp, Message_MoveTo, data[0], 0);
	}
	else if(data[0] == 4){
		SendEvent(SignalSourceApp, Message_MoveDoors, Door00, Door100);
	}
	else if(data[0] == 5){
		SendEvent(SignalSourceApp, Message_MoveDoors, Door100, Door00);
	}
}



