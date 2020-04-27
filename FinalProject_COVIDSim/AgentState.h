#ifndef AGENTSTATE_H
#define AGENTSTATE_H
#include <string>
#include <tuple> // for tuple 
#include <fstream>
#include <iostream>
#include "Variant.h"
#include "SimulationExecutive.h"
#include <unordered_map>
#include <vector>

// Agents highlevel States
enum SINs_States { Susceptible, Infected, NonSusceptible, Initialization };

// PreExisting
enum PreExisting { Cardiovascular, Diabetes, Chronic_Repriratory, Hypertension, Cancer, NoPreExisting };

// Age Variant
class AgeVariant : public Variant {
public:
	AgeVariant() {}
	double GetRV()
	{
		//https://www.census.gov/data/tables/2018/demo/age-and-sex/2018-age-sex-composition.html
		double rv = Uniform_0_1();;
		if (rv < 0.228877783)
			return rand() % 18;
		else if (rv < (0.228877783 + 0.355482663))
			return rand() % 26 + 18;
		else if (rv < (0.228877783 + 0.355482663 + 0.257576086))
			return rand() % 19 + 45;
		else if (rv < (0.093967291 + 0.228877783 + 0.355482663 + 0.257576086))
			return rand() % 9 + 65;
		else
			return rand() % 25 + 75;
	}
	Variant* New(ProbabilityParameters& param) {
		return new AgeVariant;
	}
};

// Variant for pre-existing
class PreExistingVariant {
public:
	static double Uniform_0_1()
	{
		return (((double)rand()) / (((double)RAND_MAX) + 1.0));
	}
	static PreExisting GetRVDisease(unsigned int age) {
		double rv;
		if (age < 18)
			return NoPreExisting;
		else if (age <= 44) {
			rv = Uniform_0_1();
			if (rv < 0.048)
				return Cardiovascular;
			else if (rv < (0.048 + 0.022))
				return Diabetes;
			else if (rv < (0.048 + 0.022 + 0.094))
				return Chronic_Repriratory;
			else if (rv < (0.048 + 0.022 + 0.094 + 0.088))
				return Hypertension;
			else if (rv < (0.048 + 0.022 + 0.094 + 0.088 + 0.018))
				return Cancer;
			else
				return NoPreExisting;
		}
		else if (age <= 64) {
			rv = Uniform_0_1();
			if (rv < 0.118)
				return Cardiovascular;
			else if (rv < (0.118 + 0.022))
				return Diabetes;
			else if (rv < (0.118 + 0.022 + 0.128))
				return Chronic_Repriratory;
			else if (rv < (0.118 + 0.022 + 0.128 + 0.344))
				return Hypertension;
			else if (rv < (0.118 + 0.022 + 0.128 + 0.344 + 0.096))
				return Cancer;
			else
				return NoPreExisting;
		}
		else if (age <= 74) {
			rv = Uniform_0_1();
			if (rv < 0.201)
				return Cardiovascular;
			else if (rv < (0.201 + 0.017))
				return Diabetes;
			else if (rv < (0.201 + 0.017 + 0.116))
				return Chronic_Repriratory;
			else if (rv < (0.201 + 0.017 + 0.116 + 0.467))
				return Hypertension;
			else if (rv < (0.201 + 0.017 + 0.116 + 0.467 + 0.189))
				return Cancer;
			else
				return NoPreExisting;
		}
		else if (age >= 75) {
			rv = Uniform_0_1();
			if (rv < 0.257)
				return Cardiovascular;
			else if (rv < (0.257 + 0.013))
				return Diabetes;
			else if (rv < (0.257 + 0.013 + 0.083))
				return Chronic_Repriratory;
			else if (rv < (0.257 + 0.013 + 0.083 + 0.422))
				return Hypertension;
			else if (rv < (0.257 + 0.013 + 0.083 + 0.422 + 0.215))
				return Cancer;
			else
				return NoPreExisting;
		}
	}
};



class Death_Age_HealthCondition_Prob : public Probability {
public:
	Death_Age_HealthCondition_Prob() {
	}
	float GetProb() {
		return 0;
	}
	float GetProb(float x) {
		return 0;
	}
	virtual float HealthConditionGetProb(PreExisting health) {
		switch (health)
		{
		case Cardiovascular:
			return 0.105;
			break;
		case Diabetes:
			return 0.073;
			break;
		case Chronic_Repriratory:
			return 0.063;
			break;
		case Hypertension:
			return 0.060;
			break;
		case Cancer:
			return 0.056;
			break;
		case NoPreExisting:
			return 0.09;
			break;
		default:
			break;
		}
		return 0;
	}
	virtual float AgeGetProb(unsigned int age) {
		if (age <= 9)
			return 0;
		else if (age <= 39)
			return 0.002;
		else if (age <= 49)
			return 0.004;
		else if (age <= 59)
			return 0.013;
		else if (age <= 69)
			return 0.036;
		else if (age <= 79)
			return 0.08;
		else
			return 0.148;
	}
	virtual Probability* New(ProbabilityParameters& p) {
		return new Death_Age_HealthCondition_Prob;
	}
};

class Recovered_Age_HealthCondition_Prob : public Death_Age_HealthCondition_Prob {
	float HealthConditionGetProb(PreExisting health) {
		return 1 - _da.HealthConditionGetProb(health);
	}
	virtual float AgeGetProb(unsigned int age) {
		return 1 - _da.AgeGetProb(age);
	}
	Probability* New(ProbabilityParameters& p) {
		return new Recovered_Age_HealthCondition_Prob;
	}
private:
	Death_Age_HealthCondition_Prob _da;
};

// Location Data Structure
struct Location {
	unsigned int _x;
	unsigned int _y;
	unsigned int _envID;
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

class Distance : public Parameter {
public:
	Distance(unsigned int numDistance) : Parameter{2} {
		_distance = DBG_NEW float[numDistance];				// Para 1
		_lowLevelState = DBG_NEW std::string[numDistance]; // para 2
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
		delete[] _distance;
		delete[] _lowLevelState;
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

		// Gets low level that is defined by the developer
		std::string GetLowLevelState() { return _lowLevelState; }

		// Gets the agents low level state (infected, Susceptible, or other state)
		SINs_States GetHighLevelState() { return _highLevelState; }

		// Setting Low Level State
		void SetLowLevelState(std::string state) { _lowLevelState = state; }

		virtual ~AgentStateEventAction() {}
	protected:
		Agent* _a;
		SINs_States _highLevelState{ Initialization };
		std::string _lowLevelState;

		// Next States
		std::tuple < std::string, Probability*, Variant*>* _nextStates;
		unsigned int _numNextStates;

		// Statistics
		unsigned int* _stateStat;
	};


	// Constructor for initial postiion, first state event, and age of the agent
	Agent(Location loc, AgentStateEventAction* aea, unsigned int age);

	// Runs State Transition Function pointer to determine next state transition to occur
	void AgentInteraction(Parameter* list);

	// Gets low level that is defined by the developer
	std::string GetLowLevelState() { return _agentState->GetLowLevelState(); }

	// Gets the agents low level state (infected, Susceptible, or other state)
	SINs_States GetHighLevelState() { return _agentState->GetHighLevelState(); }

	// Get location
	Location& GetLocation() { return _location; }

	// Setting Location
	void SetLocation(Location& loc);

	// Setting the event schedule for events
	void SetScheduled(bool sch) { _transitionScheduled = sch; }
	
	// Get Agent Id
	unsigned int GetId() { return _id; }

	// Get Age
	unsigned int GetAge() { return _age; }

	// Get the healthy states 
	PreExisting GetPreExisting() {
		return _PreExisting;
	}

	// Destructor
	~Agent() {
		delete _agentState;
	}
private:
	// state Variables
	Location _location;
	unsigned int _age;
	unsigned int _id;
	PreExisting _PreExisting; // Cardiovascular, Diabetes, Chronic Respiratory, Hypertension, Cancer
	static unsigned int _nextId;
	AgentStateEventAction* _agentState;
	
	// Calculation parameter
	bool _transitionScheduled;
};

typedef Agent::AgentStateEventAction StateEvent;

// Registers all states with their events
class StateEventMap{
public:
	// Singleton implementation - Static implement would also work
	static StateEventMap * GetInstance(){
		if (!_instance)
			_instance = new StateEventMap;
		return _instance;
	}

	// Registering statesEvent
	bool RegisterStateEvent(StateEvent * newRef){
		// Registering stateEvent low-lvl name and ref to stateEvnet
		_stateEventMap.push_back(newRef);
		return true;
	}

	StateEvent * GetStateEvent(std::string stateName){
		//// Finding event based on state name		
		for (int j = 0; j < _stateEventMap.size(); j++)
		{
			if (_stateEventMap[j]->GetLowLevelState() == stateName)
				return _stateEventMap[j];
		}
		// if stateName was not found
		std::cout << "\a ERROR Finding Event\n";
		exit(0); // terminate program
		return nullptr;
	}
private:
	static StateEventMap * _instance;
	StateEventMap(){ }

	// Map from stateEvent name to StateEvent ref
	std::vector<StateEvent*> _stateEventMap;
};


// State Mapping
class StateMap {
private:
	std::tuple < std::string, Probability *, Variant*>** _nextState;
	Agent::AgentStateEventAction** _newAgentEvents;
	std::string* _currStates;
	unsigned int _stateIndex, _eventIndex;
	unsigned int* _numNextStates;
	unsigned int _numStates;
	static StateMap* _instance;
public:
	StateMap() : _nextState(nullptr), _newAgentEvents(nullptr), _currStates(nullptr), _stateIndex(0), _eventIndex(0), _numNextStates(nullptr) {}

	static StateMap* GetInstance() {
		if (_instance == nullptr)
			_instance = DBG_NEW StateMap;
		return _instance;
	}

	void Initialize(unsigned int numStates) {
		_currStates = DBG_NEW std::string[numStates];
		_newAgentEvents = DBG_NEW Agent::AgentStateEventAction * [numStates];
		_nextState = DBG_NEW std::tuple < std::string, Probability*, Variant*> * [numStates];
		_numNextStates = DBG_NEW unsigned int[numStates];
		_numStates = numStates;
	}

	void RegisterStateToNextState(std::string currState, std::tuple < std::string, Probability*, Variant *>* nextStates, unsigned int numNextStates) {
		// Registering the state name
		_currStates[_stateIndex] = currState;

		_nextState[_stateIndex] = nextStates;

		// Registering the number of next states
		_numNextStates[_stateIndex] = numNextStates;

		_stateIndex++;
	}

	// Register all events at once
	void RegisterStatesToEvents_AllEvents(Agent::AgentStateEventAction** newAgentEvents) {
		// Registering events associated with states 
		_newAgentEvents = newAgentEvents;
		_eventIndex = _numStates;
	}

	// Registers event one at a time
	void RegisterStatesToEvents_IndividualEvent(Agent::AgentStateEventAction* newAgentEvents) {
		// Registering events associated with states 
		_newAgentEvents[_eventIndex] = newAgentEvents;
		_eventIndex++;
	}

	std::tuple < std::string, Probability*, Variant*>* GetNextStates(std::string currState, unsigned int& numPairs) {
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

	Agent::AgentStateEventAction* GetAgentEventAction(std::string State, Agent * a) {
		for (int i = 0; i < _numStates; i++) {
			if (_currStates[i] == State)
				return _newAgentEvents[i]->New(a);
		}
		printf("ERROR at GetAgentEventAction\a\n");
		exit(0);
	}
protected:
	~StateMap() {
		delete[] _currStates;
		delete[] _newAgentEvents;
		delete[] _nextState;
		delete[] _numNextStates;
	}
};

// Holds statistics from simulation
class STAT {
public:
	// Variables Trackers
	unsigned int _numInfected;
	unsigned int _numSusceptible;
	unsigned int _numNonSusceptible;
	unsigned int _sample;

	// Application Specific
	unsigned int _deadAge;
	unsigned int _recoveryAge;
	unsigned int _numDead;
	unsigned int _numRecovered;

	// Output Files
	std::ofstream _agentTracker;
	std::ofstream _statisticsFile;			// Output Statistics at the end of each simulation
	
	// Singleton
	static STAT* GetInstance() {
		if (_instance == nullptr)
			_instance = DBG_NEW STAT;
		return _instance;
	}

	void CumStatistics() {
		if (_sample == 0) {
			_statisticsFile << "Sample,NumSusceptible,NumInfected,NumRecovered,NumDead,CumAgeDead,CumAgeRecovery,Time" << std::endl;
		}
		_sample++;
		_statisticsFile << _sample << "," << _numSusceptible << "," << _numInfected << "," << _numRecovered << "," << _numDead << "," << _deadAge << ","
			<< _recoveryAge << "," << GetSimulationTime() << std::endl;
	}

	// Prints Agent Info
	void AgentTrackerStatistics(Agent * a);

	// Resets all variables
	void ResetSTAT() {
		_numInfected = 0;
		_numSusceptible = 0;
		_numNonSusceptible = 0;
		_numDead = 0;
		_numRecovered = 0;
		_deadAge = 0;
		_recoveryAge = 0;
	}

	// Tracks recovery and whether that caused terminations
	void AddRecovery(unsigned int age) {
		_numRecovered++;
		_recoveryAge += age;
	}

	void AddDeath(unsigned int age) {
		_numDead++;
		_deadAge += age;
	}
private:
	static STAT* _instance;
	STAT() {
		_numInfected = 0;
		_numSusceptible = 0;
		_numNonSusceptible = 0;
		_numDead = 0;
		_numRecovered = 0;
		_sample = 0;
		_deadAge = 0;
		_recoveryAge = 0;
		//std::ofstream Environment::_SIRoutputCompress = std::ofstream("OutputData//SINs_outputCompress.txt");
		//std::ofstream Environment::_SEIRoutputCompressed = std::ofstream("OutputData//SESRD_outputCompressed.txt");
		_statisticsFile = std::ofstream("OutputData//Infection_Statistics.csv");
		_agentTracker = std::ofstream("OutputData//AgentTrackerData.csv");
	}
};
#endif

