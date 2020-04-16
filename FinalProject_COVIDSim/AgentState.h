#ifndef AGENTSTATE_H
#define AGENTSTATE_H
#include <string>
#include <tuple> // for tuple 
#include <fstream>
#include <iostream>
#include "Distribution.h"
#include "SimulationExecutive.h"


// Agents highlevel States
enum SINs_States { Susceptible, Infected, NonSusceptible, Initialization };

// Holds statistics from simulation
class STAT {
public:
	static unsigned int _numInfected;
	static unsigned int _numSusceptible;
	static unsigned int _numOther;
	static void printSTAT(std::ofstream& outfile) {
		using namespace std;
		outfile << _numSusceptible << "," << _numInfected << "," << _numOther << "," << GetSimulationTime() << endl;
		//outfile << "\tTally: { S = " << _numSusceptible << " , I = " << _numInfected << " , R = " << _numOther << " | Time " << GetSimulationTime() << endl << endl;
		
		/*printf("Tally: { S = %i , I = %i , R = %i } | Time %f\n", _numSusceptible, _numInfected, _numOther, GetTime());*/
	}
};

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
	// Any disease influences that need to be updated every step in the simulation
	virtual void UpdateInfluences() = 0;
};

// Parameter Object to calculate probability of state transition
class Parameter {
public:
	Parameter(unsigned int numPara) : _numParameters(numPara) {}
	unsigned int GetNumParameters() { return _numParameters; }
protected:
	unsigned int _numParameters;
};
class Distance : public Parameter {
public:
	Distance(unsigned int numDistance) : Parameter{2} {
		_distance = new float[numDistance];				// Para 1
		_lowLevelState = new std::string[numDistance]; // para 2
		_numDistances = numDistance;
		_index = 0;
	}
	void AddDistance(float dist, std::string state) {
		_distance[_index] = dist;
		_lowLevelState[_index] = state;
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
		//delete _highLevelState;
	}
private:
	float* _distance;
	// allows for different levels of exposer pending on the state of infection (state)
	std::string* _lowLevelState;
	unsigned int _numDistances;
	unsigned int _index;
};

// Agents
class Agent {
public:
	class AgentStateEventAction : public EventAction {
	public:
		// General Execute
		void Execute();

		// Specific Execute
		virtual void StateSpecificProcess() = 0;

		// New Function
		virtual AgentStateEventAction* New() = 0;
		virtual AgentStateEventAction* New(Agent* a) = 0;

		// Determines the next state to transition too (bool will state whether it transition was scheduled)
		virtual	bool StateInteractionProcess(Parameter* list);

		// Setting Agent variable
		void SetAgent(Agent* a) { _a = a; }

		// Setting Diesease Influence
		//static void SetDiseaseInfluence(DiseaseInfluence* DI) { _dI = DI; }

		// Gets low level that is defined by the developer
		std::string GetLowLevelState() { return _lowLevelState; }

		// Gets the agents low level state (infected, Susceptible, or other state)
		SINs_States GetHighLevelState() { return _highLevelState; }

		// Setting High Level State (Infected, Susceptible, or other) and setting statistics
		void SetHighLevelState(SINs_States subState);

		void InitialSetHighLevelState();

		// Setting Low Level State
		void SetLowLevelState(std::string state) { _lowLevelState = state; }
	protected:
		Agent* _a;
		SINs_States _highLevelState{ Initialization };
		std::string _lowLevelState;
		//static DiseaseInfluence* _dI;

		// Next States
		std::tuple < std::string, Probability*, Distribution*>* _nextStates;
		unsigned int _numNextStates;
	};


	// Constructor for initial postiion, first state event, and age of the agent
	Agent(Location& loc, AgentStateEventAction* aea, unsigned int age);

	// Runs State Transition Function pointer to determine next state transition to occur
	void AgentInteraction(Parameter* list);

	// Gets low level that is defined by the developer
	std::string GetLowLevelState() { return _agentState->GetLowLevelState(); }

	// Gets the agents low level state (infected, Susceptible, or other state)
	SINs_States GetHighLevelState() { return _agentState->GetHighLevelState(); }

	// Get location
	Location& GetLocation() { return _location; }

	// Setting Location
	void SetLocation(Location& loc) { _location = loc; }

	// Setting the event schedule for events
	void SetScheduled(bool sch) { _transitionScheduled = sch; }
	
	// Get Agent Id
	unsigned int GetId() { return _id; }

private:
	// state Variables
	Location _location;
	unsigned int _age;
	unsigned int _id;
	static unsigned int _nextId;
	AgentStateEventAction* _agentState;
	
	// Calculation parameter
	bool _transitionScheduled;
};

// SusceptibleStateEvents
class SusceptibleStateEvent : public Agent::AgentStateEventAction {
public:
	SusceptibleStateEvent() { 
		_a = nullptr; 
		_highLevelState = Susceptible;
		_lowLevelState = "Susceptible";
	}

	SusceptibleStateEvent(Agent* a) { 
		_a = a; 
		_highLevelState = Susceptible;
		_lowLevelState = "Susceptible";
	}

	AgentStateEventAction* New(Agent* a) { return new SusceptibleStateEvent(a); }
	AgentStateEventAction* New() { return new SusceptibleStateEvent; }

	virtual void StateSpecificProcess();

	virtual bool StateInteractionProcess(Parameter* list);
};

// InfectedStateEvents
class InfectedStateEvent : public Agent::AgentStateEventAction {
public:
	InfectedStateEvent() { 
		_a = nullptr;  
		_highLevelState = Infected;
		_lowLevelState = "Infected";
	}

	InfectedStateEvent(Agent* a) { 
		_a = a;
		_highLevelState = Infected;
		_lowLevelState = "Infected";
	}

	AgentStateEventAction* New(Agent* a) { return new InfectedStateEvent(a); }
	AgentStateEventAction* New() { return new InfectedStateEvent; }

	virtual void StateSpecificProcess();
};

// NonSusceptibleStateEvents
class NonSusceptibleStateEvent : public Agent::AgentStateEventAction {
public:
	NonSusceptibleStateEvent() {
		_a = nullptr;
		_highLevelState = NonSusceptible;
		_lowLevelState = "NonSusceptible";
	}

	NonSusceptibleStateEvent(Agent* a) { 
		_a = a;
		_highLevelState = NonSusceptible;
		_lowLevelState = "NonSusceptible";
	}

	AgentStateEventAction* New(Agent* a) { return new NonSusceptibleStateEvent(a); }
	AgentStateEventAction* New() { return new NonSusceptibleStateEvent; }

	virtual void StateSpecificProcess();
};

// State Mapping
class StateMap {
private:
	std::tuple < std::string, Probability *, Distribution*>** _nextState;
	float** _probabilities;
	Agent::AgentStateEventAction** _newAgentEvents;
	std::string* _currStates;
	unsigned int _stateIndex;
	unsigned int* _numNextStates;
	unsigned int _numStates;
	static StateMap* _instance;
public:
	StateMap() : _nextState(nullptr), _newAgentEvents(nullptr), _currStates(nullptr), _stateIndex(0), _numNextStates(nullptr), _probabilities(nullptr) {}

	static StateMap* GetInstance() {
		if (_instance == nullptr)
			_instance = new StateMap;
		return _instance;
	}

	void Initialize(unsigned int numStates) {
		_currStates = new std::string[numStates];
		_newAgentEvents = new Agent::AgentStateEventAction * [numStates];
		_nextState = new std::tuple < std::string, Probability*, Distribution*> * [numStates];
		_numNextStates = new unsigned int[numStates];
		_probabilities = new float*[numStates];
		_numStates = numStates;
	}

	void RegisterStateToNextState(std::string currState, std::tuple < std::string, Probability*, Distribution *>* nextStates, unsigned int numNextStates) {
		// Registering the state name
		_currStates[_stateIndex] = currState;

		_nextState[_stateIndex] = nextStates;

		// Registering the number of next states
		_numNextStates[_stateIndex] = numNextStates;

		_stateIndex++;
	}

	void RegisterStatesToEvents(Agent::AgentStateEventAction** newAgentEvents) {
		// Registering events associated with states 
		_newAgentEvents = newAgentEvents;
	}

	std::tuple < std::string, Probability*, Distribution*>* GetNextStates(std::string currState, unsigned int& numPairs) {
		for (int i = 0; i < _numStates; i++)
		{
			if (_currStates[i] == currState)
			{
				// Number of pairs 
				numPairs = _numNextStates[i];

				// returning the pairs
				return _nextState[i];
			}
		}
		printf("ERROR at GetNextStates\a\n");
		exit(0);
	}

	Agent::AgentStateEventAction* GetAgentEventAction(std::string State) {
		for (int i = 0; i < _numStates; i++) {
			if (_currStates[i] == State)
				return _newAgentEvents[i];
		}
		printf("ERROR at GetAgentEventAction\a\n");
		exit(0);
	}
};
#endif

