#pragma once
#include <iostream>
#include<fstream>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

class StatData
{
public:
	static std::ofstream _statData;
};


typedef double Time;

class EventAction
{
public:
	EventAction() {};
	virtual void Execute() = 0;
};

Time GetSimulationTime();
void RunSimulation(Time endTime);
void RunSimulation(int numEvExe);
void ScheduleEventIn(Time delta, EventAction *ea);
void ScheduleEventAt(Time time, EventAction *ea);



//Testing Functionality
void FlipSteady(); //Used for testing, default steady set to false;
