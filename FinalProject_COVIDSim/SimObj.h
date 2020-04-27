#ifndef SIMOBJ_H
#define SIMOBJ_H
#include "..\..\..\GitHub\SimulationExecutive\SimulationExecutive.h"

typedef double Time;

Time GetCurrentSimTime();
void RunSimulation();
void RunSimulation(Time endTime);
void InitSimulation();

class SimObj
{
public:
	friend Time GetCurrentSimTime() {
		return sim->GetTime();
	}
	friend void RunSimulation() {
		sim->RunSim();
	}
	friend void RunSimulation(Time endTime) {
		sim->RunSim(endTime);
	}
	friend void InitSimulation() {
		sim->InitSimulation();
	}
protected:
	SimObj();
	void ScheduleEventIn(Time time, EventAction* cp) {
		sim->ScheduleEventIn(time, cp);
	}
	void ScheduleEventAt(Time time, EventAction* cp) {
		sim->ScheduleEventAt(time, cp);
	}
private:
	static SimulationExecutive* sim;
};
#endif
