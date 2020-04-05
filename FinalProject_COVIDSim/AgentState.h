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
	unsigned int _numParameters{ 0 };
};
class Distance : public Parameter {
public:
	float* _distance = nullptr;
	unsigned int _numDistances = 0;
	unsigned int _index = 0;
	Distance(unsigned int numDistance) {
		_distance = new float[numDistance];
		_numDistances = numDistance;
		_numParameters = 1;
	}
	void AddDistance(float dist) {
		_distance[_index] = dist;
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
// event to transition to this state
class HealthyEvent : public EventAction {
public:
	HealthyEvent(Agent* a) { _a = a; }
	class HealthyState : public Agent::AgentState {
	public:
		HealthyState() { _highLevelState = Healthy; _lowLevelState = "Healthy"; }
		virtual void StateTransition(Agent* a); // Will determine the next state and call event
	};
	// Execute Event
	void Execute();
private:
	Agent* _a;

};

// Event to transition to this state
class StandardHealthEvent : public EventAction {
public:
	StandardHealthEvent(Agent* a) : _a(a) {}
	// State In Accordance with event
	class StandardHealthyState : public HealthyEvent::HealthyState {
	public:
		StandardHealthyState() { _lowLevelState = "Standard_Health"; }
		void StateTransition(Agent* a);
	};
	void Execute();
private:
	Agent* _a;
};

//-----------------------Infected States-------------------------------------
// Event to transition to this state
class InfectionEvent : public EventAction {
public:
	InfectionEvent(Agent* a);
	class InfectedState : public Agent::AgentState {
	public:
		InfectedState() { _highLevelState = Infected; _lowLevelState = "Infected"; }
		virtual void StateTransition(Agent* a);
	};
	void Execute();
};

//-----------------------Other States-------------------------------------
class OtherState : public Agent::AgentState {
public:
	OtherState() {
		_highLevelState = Other;
	}

	// Event to transition to this State
	class Event : public EventAction{
	public:
		Event(Agent* a);

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
	class Event : public EventAction {
	public:
		Event(Agent* a);

		void Execute();
	};

	virtual void StateTransition(Agent* a);

};
#endif

