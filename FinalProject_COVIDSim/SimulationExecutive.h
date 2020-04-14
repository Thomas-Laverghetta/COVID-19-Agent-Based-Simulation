#pragma once
#include <iostream>
#include<fstream>


using namespace std;

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
