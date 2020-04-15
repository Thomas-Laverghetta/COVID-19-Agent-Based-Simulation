#ifndef AGENTSTATE_H
#define AGENTSTATE_H
#include <string>
#include <tuple> // for tuple 
#include <fstream>
#include <iostream>
#include "Distribution.h"
#include "SimulationExecutive.h"


// Agents highlevel States
enum SIR_States { Susceptible, Infected, NonSusceptible, Initialization };

// Holds statistics from simulation
class STAT {
public:
	static unsigned int _numInfected;
	static unsigned int _numSusceptible;
	static unsigned int _numOther;
	static void printSTAT(std::ofstream& outfile) {
		using namespace std;
		outfile << "\tTally: { S = " << _numSusceptible << " , I = " << _numInfected << " , R = " << _numOther << " | Time " << GetSimulationTime() << endl << endl;
		
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
		//delete _highLevelState;
	}
private:
	float* _distance;
	// allows for different levels of exposer pending on the state of infection (state)
	std::string* _highLevelState;
	unsigned int _numDistances;
	unsigned int _index;
};

// Agents
class Agent {
public:
	class AgentEventAction : public EventAction {
	public:
		// General Execute
		void Execute();

		// Specific Execute
		virtual void Execute2() = 0;

		// New Function
		virtual AgentEventAction* New() = 0;
		virtual AgentEventAction* New(Agent* a) = 0;

		// Determines the next state to transition too (bool will state whether it transition was scheduled)
		virtual	bool StateTransitionProcess(Parameter* list) = 0;

		// Setting Agent variable
		void SetAgent(Agent* a) { _a = a; }

		//// Setting List
		//void SetParameterList(Parameter* list) {
		//	_list = list;
		//}

		// Setting Diesease Influence
		//static void SetDiseaseInfluence(DiseaseInfluence* DI) { _dI = DI; }

		// Gets low level that is defined by the developer
		std::string GetLowLevelState() { return _lowLevelState; }

		// Gets the agents low level state (infected, Susceptible, or other state)
		SIR_States GetHighLevelState() { return _highLevelState; }

		// Setting High Level State (Infected, Susceptible, or other) and setting statistics
		void SetHighLevelState(SIR_States subState);

		void InitialSetHighLevelState();

		// Setting Low Level State
		void SetLowLevelState(std::string state) { _lowLevelState = state; }
	protected:
		Agent* _a;
		SIR_States _highLevelState{ Initialization };
		std::string _lowLevelState;
		//static DiseaseInfluence* _dI;

		// Next States
		std::pair<std::string, Distribution*>* _nextStates;
		float* _nextProbabilities;
		unsigned int _numNextStates;
	};


	// Constructor for initial postiion, first state event, and age of the agent
	Agent(Location& loc, AgentEventAction* aea, unsigned int age);

	// Runs State Transition Function pointer to determine next state transition to occur
	void StateTransition(Parameter* list);

	// Gets low level that is defined by the developer
	std::string GetLowLevelState() { return _aea->GetLowLevelState(); }

	// Gets the agents low level state (infected, Susceptible, or other state)
	SIR_States GetHighLevelState() { return _aea->GetHighLevelState(); }

	// Setting High Level State (Infected, Susceptible, or other) and setting statistics
	void SetHighLevelState(SIR_States subState) { _aea->SetHighLevelState(subState); }

	// Setting Low Level State
	void SetLowLevelState(std::string state) { _aea->SetLowLevelState(state); }

	//// Parameters (e.g., Distance)
	//void SetParameters(Parameter* list);

	// Get location
	Location& GetLocation() { return _location; }

	// Setting Location
	void SetLocation(Location& loc) { _location = loc; }

	// Setting the event schedule for events
	void SetScheduled(bool sch) { _scheduled = sch; }
	
	// Get Agent Id
	unsigned int GetId() { return _id; }

	// Setting Curr State Event
	void SetAgentEventAction(AgentEventAction* aea);
private:
	// state Variables
	Location _location;
	unsigned int _age;
	unsigned int _id;
	static unsigned int _nextId;
	AgentEventAction* _aea;
	
	// Calculation parameter
	bool _scheduled;
};

// SusceptibleStateEvents
class SusceptibleStateEvent : public Agent::AgentEventAction {
public:
	SusceptibleStateEvent() { 
		_a = nullptr; 
		_highLevelState = Susceptible;
		_lowLevelState = "Susceptible";
		//_numProbabilities = 1;
	}

	SusceptibleStateEvent(Agent* a) { 
		_a = a; 
		_highLevelState = Susceptible;
		_lowLevelState = "Susceptible";
		//_numProbabilities = 1;
	}

	AgentEventAction* New(Agent* a) { return new SusceptibleStateEvent(a); }
	AgentEventAction* New() { return new SusceptibleStateEvent; }

	virtual void Execute2();

	bool StateTransitionProcess(Parameter* list);
private:
	static float _expDistributionRate;
};

// InfectedStateEvents
class InfectedStateEvent : public Agent::AgentEventAction {
public:
	InfectedStateEvent() { 
		_a = nullptr;  
		_highLevelState = Infected;
		_lowLevelState = "Infected";
		//_numProbabilities = 0;
	}

	InfectedStateEvent(Agent* a) { 
		_a = a;
		_highLevelState = Infected;
		_lowLevelState = "Infected";
	/*	_numProbabilities = 0;*/
	}

	AgentEventAction* New(Agent* a) { return new InfectedStateEvent(a); }
	AgentEventAction* New() { return new InfectedStateEvent; }

	virtual void Execute2();

	bool StateTransitionProcess(Parameter* list);
};

// NonSusceptibleStateEvents
class NonSusceptibleStateEvent : public Agent::AgentEventAction {
public:
	NonSusceptibleStateEvent() {
		_a = nullptr;
		_highLevelState = NonSusceptible;
		_lowLevelState = "NonSusceptible";
		//_numProbabilities = 0;
	}

	NonSusceptibleStateEvent(Agent* a) { 
		_a = a;
		_highLevelState = NonSusceptible;
		_lowLevelState = "NonSusceptible";
		/*_numProbabilities = 0;*/
	}

	AgentEventAction* New(Agent* a) { return new NonSusceptibleStateEvent(a); }
	AgentEventAction* New() { return new NonSusceptibleStateEvent; }

	virtual void Execute2();

	bool StateTransitionProcess(Parameter* list);
};

// State Mapping
class StateMap {
private:
	std::pair <std::string, Distribution *>** _nextState;
	float** _probabilities;
	Agent::AgentEventAction** _newAgentEvents;
	std::string* _stateArray;
	unsigned int _stateIndex;
	unsigned int* _numNextStates;
	unsigned int _numStates;
	static StateMap* _instance;
public:
	StateMap() : _nextState(nullptr), _newAgentEvents(nullptr), _stateArray(nullptr), _stateIndex(0), _numNextStates(nullptr), _probabilities(nullptr) {}

	static StateMap* GetInstance() {
		if (_instance == nullptr)
			_instance = new StateMap;
		return _instance;
	}

	void Initialize(unsigned int numStates) {
		_stateArray = new std::string[numStates];
		_newAgentEvents = new Agent::AgentEventAction * [numStates];
		_nextState = new std::pair < std::string, Distribution *>* [numStates];
		_numNextStates = new unsigned int[numStates];
		_probabilities = new float*[numStates];
		_numStates = numStates;
	}

	void RegisterStateToNextState(std::string currState, std::tuple < std::string, float, Distribution *>* nextStates, unsigned int numNextStates) {
		// Registering the state name
		_stateArray[_stateIndex] = currState;

		_nextState[_stateIndex] = new std::pair < std::string, Distribution*> [numNextStates];
		_probabilities[_stateIndex] = new float[numNextStates];

		// copy information
		for (int i = 0; i < numNextStates; i++) {
			// registering next states
			_nextState[_stateIndex][i].first = std::get<0>(nextStates[i]);
			_nextState[_stateIndex][i].second = std::get<2>(nextStates[i]);
			_probabilities[_stateIndex][i] = std::get<1>(nextStates[i]);
		}

		// Registering the number of next states
		_numNextStates[_stateIndex] = numNextStates;

		_stateIndex++;
	}

	void RegisterStatesToEvents(Agent::AgentEventAction** newAgentEvents) {
		// Registering events associated with states 
		_newAgentEvents = newAgentEvents;
	}

	std::pair<std::string, Distribution *>* GetNextStates(std::string currState, unsigned int& numPairs, unsigned int& j) {
		for (int i = 0; i < _numStates; i++)
		{
			if (_stateArray[i] == currState)
			{
				j = i;

				// Number of pairs 
				numPairs = _numNextStates[i];

				//// copy probabilities
				//probabilities = new float[numPairs];
				//for (int j = 0; j < numPairs; j++)
				//	probabilities[j] = _probabilities[i][j]; // full copy of values


				// returning the pairs
				return _nextState[i];
			}
		}
		printf("ERROR at GetNextStates\a\n");
		exit(0);
	}

	float* GetProbabilities(unsigned int i) {
		// copy probabilities
		float * probabilities = new float[_numNextStates[i]];
		for (int j = 0; j < _numNextStates[i]; j++)
			probabilities[j] = _probabilities[i][j]; // full copy of values

		return probabilities;
	}
	Agent::AgentEventAction* GetAgentEventAction(std::string nextState) {
		for (int i = 0; i < _numStates; i++) {
			if (_stateArray[i] == nextState)
				return _newAgentEvents[i];
		}
		printf("ERROR at GetAgentEventAction\a\n");
		exit(0);
	}
};
#endif

