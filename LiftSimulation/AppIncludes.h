/*
 * AppIncludes.h
 *
 * Created: 09.12.2019 09:52:06
 *  Author: rolfl
 */ 


#ifndef APPINCLUDES_H_
#define APPINCLUDES_H_
#include "LiftLibrary.h"

typedef enum AppMessages
{
	Message_MoveTo = 1,	
	Message_PosChanged = 2,
	Message_ElevatorReady = 3,
	Message_MoveDoors = 10,
	Message_DoorsOpened = 11
};

void MotorCtrl_Initializing(Message* msg);
void MotorCtrl_Stopped(Message* msg);
void MotorCtrl_Moving(Message* msg);
void MotorCtrl_DoorsMoving(Message* msg);
void MotorCtrl_DoorsOpened(Message* msg);


typedef struct MotorController_tag
{
	Fsm fsm;
	FloorType start;
	FloorType target;
} MotorController;

extern MotorController _motorCtrl;

void MainCtrl_Initializing(Message* msg);
void MainCtrl_AwaitElevatorRequest(Message* msg);
void MainCtrl_AwaitTargetSelection(Message* msg);
void MainCtrl_ElevatorMoving(Message* msg);

typedef struct MainCtrl_tag
{
	Fsm fsm;							// Zustandsmaschine
	FloorType currentFloor;				// aktuelle Position
	FloorType nextFloor;				// nächste Zielposition
	uint8_t timer;						// erlaubt den Timer index zu speichern
	uint8_t pendingRequests;			// Bitmaske um mehrfach einträge in die Q zu vermeiden
	FloorType ElevatorNextPosQ[4];		// q für reservationen
	uint8_t qIn;						// position für nächsten put
	uint8_t qOut;                       // position für nächsten get
}MainCtrl;

extern MainCtrl _mainCtrl;


#endif /* APPINCLUDES_H_ */