#ifndef AGENTSTATE_H
#define AGENTSTATE_H
#include <string>
#include "SimObj.h"
#include "Distribution.h"

// STAT
class STAT {
public:
	static unsigned int _numInfected;
	static unsigned int _numSusceptible;
	static unsigned int _numOther;
	static void printSTAT() {
		printf("S = %i | I = %i | R = %i | ", _numSusceptible, _numInfected, _numOther);
	}
};

// Agents highlevel States
enum SIR_States { Susceptible, Infected, Other };

// Location Data Structure
struct Location {
	unsigned int _x;
	unsigned int _y;
};

/* Disease Influence
	Allows users to create disease influences to use during state transitions

	e.g., temp, climate, etc.
*/
class DiseaseInfluence {
public:
	virtual void UpdateInfluences() = 0;
};

// Parameter Object to calculate probability of state transition
class Parameter {
public:
	Parameter(unsigned int numPara) : _numParameters(numPara) {}
protected:
	unsigned int _numParameters;
};
class Distance : public Parameter {
public:
	Distance(unsigned int numDistance) : Parameter{2} {
		_distance = new float[numDistance];				// Para 1
		_highLevelState = new std::string[numDistance]; // para 2
		_numDistances = numDistance;
		_numParameters = 1;
		_index = 0;
	}
	void AddDistance(float dist, std::string state) {
		_distance[_index] = dist;
		_highLevelState[_index] = state;
		_index++;
	}
	void resetIndex() {
		_index = 0;
	}
	unsigned int size() { return _numDistances; }
	float operator[](unsigned int index) {
		if (index <= index)
			return _distance[index];
		else
			return -1;
	}
	~Distance() {
		delete _distance;
	}
private:
	float* _distance;
	// allows for different levels of exposer pending on the state of infection (state)
	std::string* _highLevelState;
	unsigned int _numDistances;
	unsigned int _index;
};

// Agents
class Agent : public SimObj {
public:
	Agent(Location& loc, EventAction * ea, unsigned int age);

	// Runs State Transition Function pointer to determine next state transition to occur
	void StateTransition();

	// Gets low level that is defined by the developer
	std::string GetLowLevelState();

	// Gets the agents low level state (infected, Susceptible, or other state)
	SIR_States GetHighLevelState();

	// Setting High Level State (Infected, Susceptible, or other) 
	void SetHighLevelState(SIR_States subState);

	// Setting Low Level State
	void SetLowLevelState(std::string state);

	// Setting Diesease Influence
	static void SetDiseaseInfluence(DiseaseInfluence* DI);

	// Parameters (e.g., Distance)
	void SetParameters(Parameter* list);

	// Get location
	Location& GetLocation() {
		return _location;
	}

	// Setting Location
	void SetLocation(Location& loc) {
		_location = loc;
	}

	void SetScheduled(bool sch) { _scheduled = sch; }
	// Get Id
	unsigned int GetId() { return _id; }
	// Calculation Parameters
	bool (*_stateTranitionFunction)(Agent*);
	Parameter* _list;
	float* _probabilities;
private:
	// state Variables
	Location _location;
	unsigned int _age;
	unsigned int _id;
	static DiseaseInfluence* _DI;
	SIR_States _highLevelState;
	std::string _lowLevelState;
	static unsigned int _nextId;
	bool _scheduled;
};

//typedef float* (*StateTransitionFunction)(Agent*);
typedef bool (*StateTransitionFunction)(Agent*);
typedef EventAction* (*NextEvents)(Agent *);

class AgentEventAction : public EventAction, public SimObj {
public:
	//virtual bool StateTransition(Agent* a) = 0;
};

class SusceptibleStateEvent : public AgentEventAction {
public:
	SusceptibleStateEvent() { _a = nullptr;  }

	SusceptibleStateEvent(Agent* a) { _a = a; }

	static EventAction* New(Agent* a) { return new SusceptibleStateEvent(a); }

	void Execute();

	void SetAgent(Agent* a) { _a = a; }

	static bool StateTransition(Agent* a);
private:
	Agent* _a;
	static float _expDistributionRate;
};

class InfectedStateEvent : public AgentEventAction {
public:
	InfectedStateEvent() { _a = nullptr;  }

	InfectedStateEvent(Agent* a) { _a = a; }

	static EventAction* New(Agent* a) { return new InfectedStateEvent(a); }

	void Execute();

	void SetAgent(Agent* a) { _a = a; }

	static bool StateTransition(Agent* a);
private:
	Agent* _a;
	static Distribution* _timeDelay;
};

class RecoveredStateEvent : public AgentEventAction {
public:
	RecoveredStateEvent(Agent* a) { _a = a; }

	static EventAction* New(Agent* a) { return new RecoveredStateEvent(a); }

	void Execute();

	static bool StateTransition(Agent* a);
private:
	Agent* _a;
};
#endif

