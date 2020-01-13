/*
* SysCtrl.c
*
* Created: 03.01.2020 09:38:45
* Author: Rolf Laich
*
* Template für die Liftsteuerungs-Zustandsmaschine
* für die Behandlung der Buttons usw
*/

#include "AppIncludes.h"

// nützliche Macros
// ****************
#define DOOR_CLOSE_TIMEOUT				20000
#define IS_BUTTON_PRESS(msg)			((msg->Id == ButtonEvent) && (msg->MsgParamHigh == Pressed))
#define IS_RESERVATION(buttonMask)		((buttonMask)&0xF0)
#define IS_TARGET_SELECTION(buttonMask)	((buttonMask)&0x0F)

#define countof(array)					(sizeof(array)/sizeof(array[0]))
#define NOT_PENDING(floor)				!(_mainCtrl.pendingRequests&(1<<floor))

// private Funktionen
FloorType GetFloorReservation(uint8_t msgParam);
FloorType GetTargetSelection(uint8_t msgParam);

void HandleButtonInput(Message* msg);

Boolean Enqueue(FloorType floor);
Boolean Dequeue(FloorType* floor);

uint8_t FindBit(uint8_t value);



MainCtrl _mainCtrl = {
	.currentFloor = Floor0,
	.nextFloor = Floor0,
	.timer = 0,
	.pendingRequests = 0,
	.fsm = {.Next = 0, .RxMask = 0xFF, .CurrentState = MainCtrl_Initializing},
	
};

//Initialisierung
void MainCtrl_Initializing(Message* msg)
{
	if( msg->Id == Message_ElevatorReady)
	{
		_mainCtrl.currentFloor = Floor0;
		EnableStatusUpdate = true;
		SetState(&_mainCtrl.fsm, MainCtrl_AwaitElevatorRequest);
		return;
	}
}

//Lift hat nichts zu tun. Türen geschlossen auf einem Stockwerk am warten
void MainCtrl_AwaitElevatorRequest(Message* msg)
{
	Usart_PutChar(0xA0);
	Usart_PutChar(msg->Id);
	
	HandleButtonInput(msg);

	if( IS_BUTTON_PRESS( msg ) )
	{
		if(Dequeue(&_mainCtrl.nextFloor)){
			SetState(&_mainCtrl.fsm, MainCtrl_ElevatorMoving);
			SendEvent(SignalSourceElevator, Message_MoveTo, _mainCtrl.nextFloor, 0);
		}
	}

}

//Lift steht auf Stockwerk mit offenen Türen
void MainCtrl_AwaitTargetSelection(Message* msg)
{
	Usart_PutChar(0xB0);
	Usart_PutChar(msg->Id);
	
	HandleButtonInput(msg);
		
	if(msg->Source == SignalSourceDoor && ReadDoorState(_mainCtrl.currentFloor) == DoorClosed){
		
		//Falls keine Aufträge mehr Vorhanden sind, in den Await Request State wechseln, sonst nächsten Auftrag abarbeiten.
		if(!Dequeue(&_mainCtrl.nextFloor)){
			SetState(&_mainCtrl.fsm, MainCtrl_AwaitElevatorRequest);
		}
		else{
			SetState(&_motorCtrl.fsm, MotorCtrl_DoorsMoving);
			SetState(&_mainCtrl.fsm, MainCtrl_ElevatorMoving);
			SendEvent(SignalSourceElevator, Message_MoveDoors, 100, 0);
		}
		
	}
	
}

//Lift in Bewegung
void MainCtrl_ElevatorMoving(Message* msg)
{
	Usart_PutChar(0xC0);
	Usart_PutChar(msg->Id);
	
	HandleButtonInput(msg);
	
	if(msg->Source == SignalSourceDoor){
		SendEvent(SignalSourceApp, Message_MoveTo, _mainCtrl.nextFloor, 0);
	}
	
	if( msg->Id == Message_PosChanged)
	{
		//Ankunft auf Stockwerk
		if( msg->MsgParamHigh == msg->MsgParamLow)
		{
			_mainCtrl.currentFloor = msg->MsgParamHigh/POS_STEPS_PER_FLOOR;
			ClrIndicatorFloorState(_mainCtrl.currentFloor); // Lampen ausschalten
			ClrIndicatorElevatorState(_mainCtrl.currentFloor);
			SetState(&_mainCtrl.fsm, MainCtrl_AwaitTargetSelection);
		}
	}
}



FloorType GetFloorReservation(uint8_t buttonEventParameter )
{
	return FindBit(buttonEventParameter) - 4;
}

FloorType GetTargetSelection(uint8_t buttonEventParameter )
{
	return FindBit(buttonEventParameter);
}

//Kann button inputs auslesen und eine Reservation erstellen. Kontrolliert auch das Einschalten der Lampen auf dem Board
void HandleButtonInput(Message* msg){
	if( IS_BUTTON_PRESS( msg ) )
	{		
		FloorType target = Floor0;
		
		if( IS_RESERVATION(msg->MsgParamLow)){
			target = GetFloorReservation(msg->MsgParamLow);
			SetIndicatorFloorState(target);
		}
		else{
			target = GetTargetSelection(msg->MsgParamLow);
			SetIndicatorElevatorState(target);
		}
	
		if( target != _mainCtrl.nextFloor && NOT_PENDING(target))
		{
			Enqueue(target);
		}	
	}
}


Boolean Enqueue(FloorType floor)
{
	uint8_t nextIn = (_mainCtrl.qIn + 1)%countof(_mainCtrl.ElevatorNextPosQ);
	if( nextIn != _mainCtrl.qOut)
	{
		_mainCtrl.ElevatorNextPosQ[_mainCtrl.qIn] = floor;
		_mainCtrl.qIn = nextIn;
		return true;
	}
	return false;
}

// out floor
Boolean Dequeue(FloorType* floor)
{
	if( _mainCtrl.qIn != _mainCtrl.qOut)
	{
		*floor = _mainCtrl.ElevatorNextPosQ[_mainCtrl.qOut];
		_mainCtrl.qOut = (_mainCtrl.qOut + 1)%countof(_mainCtrl.ElevatorNextPosQ);
		return true;
	}
	return false;
}

uint8_t FindBit(uint8_t value)
{
	uint8_t maxsize = sizeof(value) * 8;
	uint8_t i = 0;

	if (value == 0) {
		return INT32_MAX;
	}

	while ((value & 0x1) != 0x1 && i < maxsize) {
		i++;
		value = value >> 1;
	};

	return i;
}
