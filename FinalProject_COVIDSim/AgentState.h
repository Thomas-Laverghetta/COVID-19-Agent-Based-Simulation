#ifndef AGENTSTATE_H
#define AGENTSTATE_H
#include <string>
#include "SimObj.h"

// Agents highlevel States
enum SIR_States { Healthy, Infected, Other };

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
	float* _distance = nullptr;

	// allows for different levels of exposer pending on the state of infection (state)
	std::string * _highLevelState; 
	unsigned int _numDistances = 0;
	unsigned int _index = 0;
	Distance(unsigned int numDistance) : Parameter{2} {
		_distance = new float[numDistance];				// Para 1
		_highLevelState = new std::string[numDistance]; // para 2
		_numDistances = numDistance;
		_numParameters = 1;
	}
	void AddDistance(float dist, std::string state) {
		_distance[_index] = dist;
		_highLevelState[_index] = state;
		_index++;
	}
	void resetIndex() {
		_index = 0;
	}
};

// Agents
class Agent : public SimObj {
public:
	// Base for all states
	class AgentState {
	public:
		virtual void StateTransition(Agent* a) = 0;

		std::string GetAgentState();

		SIR_States GetAgentSubState();

		void SetAgentSubState(SIR_States subState);

		void SetAgentState(std::string state);

		static void SetDiseaseInfluence(DiseaseInfluence* DI);

		void SetParameters(Parameter * list);
	protected:
		SIR_States _highLevelState;
		std::string _lowLevelState;
		static DiseaseInfluence* _DI;
		Parameter* _list;
	};
	Agent(Location loc, AgentState* initialState, unsigned int age);

	// state Variables
	Location _location;
	unsigned int _age;
	unsigned int _id;
	static unsigned int _nextId;

	AgentState* _agentState;
};

//-----------------------Healthy States-------------------------------------
class HealthyState : public Agent::AgentState {
public:
	HealthyState() { _highLevelState = Healthy; _lowLevelState = "Healthy"; }
	
	// Event to transition to this state
	class StateTransitionEvent : public EventAction {
	public:
		StateTransitionEvent(Agent* a) { _a = a; }

		// Execute Event
		void Execute();
	private:
		Agent* _a;

	};
	virtual void StateTransition(Agent* a); // Will determine the next state and call event
};

// Example of Healthy state
class StandardHealthyState : public HealthyState {
public:
	StandardHealthyState() { _lowLevelState = "Standard_Health"; }

	// Event to transition to this state
	class StateTransitionEvent : public EventAction {
	public:
		StateTransitionEvent(Agent* a) { _a = a; }

		// Execute Event
		void Execute();
	private:
		Agent* _a;

	};
	void StateTransition(Agent* a);
};
//-----------------------Infected States-------------------------------------
class InfectedState : public Agent::AgentState {
public:
	InfectedState() { _highLevelState = Infected; _lowLevelState = "Infected"; }
	
	// Event to transition to this state
	class StateTransitionEvent : public EventAction {
	public:
		StateTransitionEvent(Agent* a) { _a = a; }

		// Execute Event
		void Execute();
	private:
		Agent* _a;

	};

	virtual void StateTransition(Agent* a);
};

//-----------------------Other States-------------------------------------
class OtherState : public Agent::AgentState {
public:
	OtherState() {
		_highLevelState = Other;
	}

	// Event to transition to this State
	class StateTransitionEvent : public EventAction{
	public:
		StateTransitionEvent(Agent* a);

		void Execute();
	};

	virtual void StateTransition(Agent* a);

};

class TestOtherState : public OtherState {
public:
	TestOtherState() {
		_highLevelState = Other;
	}

	// Event to transition to this State
	class StateTransitionEvent : public EventAction {
	public:
		StateTransitionEvent(Agent* a);

		void Execute();
	};

	virtual void StateTransition(Agent* a);

};
#endif

