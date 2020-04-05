#include "AgentState.h"

// Initializing Static Variables
unsigned int Agent::_nextId = 0;
Agent::Agent(Location loc, AgentState* initialState, unsigned int age)
{
	// Initializing variables
	_id = _nextId++;
	_location = loc;
	_agentState = initialState;
	_age = age;

	// Scheduling SIR Event if infected
	if (_agentState->GetAgentSubState() == Infected)
	{
		_agentState->StateTransition(this);
	}
}


DiseaseInfluence* Agent::AgentState::_DI = nullptr;


std::string Agent::AgentState::GetAgentState()
{
	return _lowLevelState;
}

SIR_States Agent::AgentState::GetAgentSubState()
{
	return _highLevelState;
}

void Agent::AgentState::SetDiseaseInfluence(DiseaseInfluence* DI)
{
	_DI = DI;
}

void Agent::AgentState::SetParameters(Parameter * list)
{
	delete _list; // deleting previous dynamic object to point to new object
	_list = list;
}



//---------------------HEALTHY STATES--------------------------

void HealthyEvent::Execute()
{
	delete _a->_agentState;
	_a->_agentState = new HealthyState;
	_a->_agentState->StateTransition(_a);
}
void HealthyEvent::HealthyState::StateTransition(Agent* a)
{
}

//// Example Healthy State
void StandardHealthEvent::Execute()
{
	delete _a->_agentState;
	_a->_agentState = new StandardHealthyState;
	_a->_agentState->StateTransition(_a);
}
void StandardHealthEvent::StandardHealthyState::StateTransition(Agent* a)
{
}




//-----------------------Infected States-------------------------------------

void InfectionEvent::InfectedState::StateTransition(Agent* a)
{
}


