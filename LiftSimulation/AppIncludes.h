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
	Message_MoveDoors = 3,
};

void MotorCtrl_Initializing(Message* msg);
void MotorCtrl_Stopped(Message* msg);
void MotorCtrl_Moving(Message* msg);
void MotorCtrl_DoorsMoving(Message* msg);


typedef struct MotorController_tag
{
	Fsm fsm;
	FloorType start;
	FloorType target;
} MotorController;

extern MotorController _motorCtrl;


#endif /* APPINCLUDES_H_ */