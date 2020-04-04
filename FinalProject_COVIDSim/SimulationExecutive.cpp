#include "SimulationExecutive.h"

SimulationExecutive* SimulationExecutive::m_instance = nullptr;

void SimulationExecutive::RunSim(Time T){
    // loops until either time expires or there is no more events
    while(m_EventSet.EventSetSize() > 0){
        // Gets the time of the next event to update the simulation time
        m_SimTime = m_EventSet.GetEventTime();

        // Time Termination
        if (((T != -1) ? T < m_SimTime : false))
            break;

        // Gets next event action and executes it
        m_EventSet.GetEventAction()->Execute();
    }
}

void SimulationExecutive::InitSimulation()
{
	// Resetting attributes
	m_SimTime = 0.0f;
	m_EventSet.~EventSet();
}

void SimulationExecutive::ScheduleEventIn(const Time& deltaT, EventAction * ea){
    m_EventSet.AddEvent(deltaT + m_SimTime, ea);
}

void SimulationExecutive::ScheduleEventAt(const Time& t, EventAction * ea){
    m_EventSet.AddEvent(t, ea);
}
