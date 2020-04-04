#ifndef SIMULATION_EXECUTIVE_H
#define SIMULATION_EXECUTIVE_H

#include "EventAction.h"
#include "EventSet.h"

/*
    This implementation will use singleton approach
*/
typedef double Time;

class SimulationExecutive
{
private:
    // Singleton Data  
    SimulationExecutive(){ m_SimTime = 0.0f;};
    ~SimulationExecutive(){};
    static SimulationExecutive * m_instance;

    // Simualation data
    Time m_SimTime;
    EventSet m_EventSet;
public:
    // SINGLETON
    static SimulationExecutive * GetInstance(){
        if(m_instance == nullptr)
            m_instance = new SimulationExecutive();
        
        return m_instance;
    }

    // Get simulation time
    Time GetTime() {return m_SimTime;}

    // Schedules event in delta-time from current sim time
    void ScheduleEventIn(const Time& deltaT, EventAction * ea);

    // Schedulees event at t sim time
    void ScheduleEventAt(const Time& t, EventAction * ea);

    /*
        runs simulation executive
        defualt value of -1 indicates run until all events executed
    */
    void RunSim(Time T = -1);

    // initializes simulation
    void InitSimulation();
};
#endif