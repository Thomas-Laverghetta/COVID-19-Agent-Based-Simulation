#include "AgentState.h"
////---------------STAT----------------
STAT* STAT::_instance = nullptr;

// Initializing Static Variables
unsigned int Agent::_nextId = 0;
StateMap* StateMap::_instance = nullptr;
StateEventMap* StateEventMap::_instance = nullptr;


Agent::Agent(Location& loc, AgentStateEventAction* aea, unsigned int age)
{
	// Initializing variables
	_id = _nextId++;
	_location = loc;
	_age = age;
	_agentState = aea;

	// Application
	_PreExisting = PreExistingVariant::GetRVDisease(_age);

	// Setting the event to this agent
	_agentState->SetAgent(this);

	// setting the initial state of the agent: High and Low states
	ScheduleEventAt(0, aea); 
}

// Agent determines whether to transition based on interactions or time
void Agent::AgentInteraction(Parameter* list)
{
	// If transition has been scheduled
	if (!_transitionScheduled) {
		// calling function to determine whether to change states and what state
		_transitionScheduled = _agentState->StateInteractionProcess(list);
	}
}

//----------------------------MASTER_EXECUTE-------------------------
/* 
	Every State transfer event will have the same general protocol
	1. Setting High and Low level state
	2. Resetting schedule
	3. Setting the number parameters
	4. Run Specific State Transfer Execute (Execute2)
		- This will allow for anything that needs to be done in the
		- event (that hasnt been already been done in Execute()).
*/
void Agent::AgentStateEventAction::Execute()
{
	// Transitioning states
	_a->_transitionScheduled = false; // RESETTING SCH IN AGENT

	// Setting Agent State
	if (this != _a->_agentState) {
		(*_a->_agentState->_stateStat)--;
		delete _a->_agentState;
		_a->_agentState = this; // gives agent's attributes (low and high level states)
	}
	(*_stateStat)++;

	// Register to get next states
	unsigned int i = 0;
	_nextStates = StateMap::GetInstance()->GetNextStates(_lowLevelState, _numNextStates);

	// Specific State Event Execution
	StateSpecificProcess();
}

bool Agent::AgentStateEventAction::StateInteractionProcess(Parameter* list)
{
	// _nextStates {string state name, float probability, Variant TimeDelay} 
	// Determining whether this is a terminating state
	if (std::get<1>(_nextStates[0]) > 0) {
		unsigned int RNG = rand() % 101;
		unsigned int i = 0;
		float temp = 0;
		do {
			temp = std::get<1>(_nextStates[i])->GetProb() + temp;
			i++;
		} while (i!= _numNextStates && temp * 100 >= RNG);
		if (temp * 100 >= RNG) {
			ScheduleEventIn(std::get<2>(_nextStates[i-1])->GetRV(), StateMap::GetInstance()->GetAgentEventAction(std::get<0>(_nextStates[i-1]), _a));
			return true;
		}
		else {
			// stay on current state
			return false;
		}
	}
	return true;
}