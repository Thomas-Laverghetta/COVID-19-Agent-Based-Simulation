#include "AgentState.h"
////---------------STAT----------------
STAT* STAT::_instance = nullptr;

// Initializing Static Variables
unsigned int Agent::_nextId = 0;

//int Agent::AgentStateEventAction::i = 0;

//DiseaseInfluence* Agent::AgentEventAction _dI = nullptr;
StateMap* StateMap::_instance = nullptr;

Agent::Agent(Location& loc, AgentStateEventAction* aea, unsigned int age)
{
	// Initializing variables
	_id = _nextId++;
	_location = loc;
	_age = age;
	_agentState = aea;

	// Application
	_PreExisting = PreExistingDistribution::GetRVDisease(_age);

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
	// _nextStates {string state name, float probability, distribution TimeDelay} 
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

//---------------------HEALTHY STATES-------------------------
void SusceptibleStateEvent::StateSpecificProcess()
{ 
}
bool SusceptibleStateEvent::StateInteractionProcess(Parameter* list)
{
	if (std::get<1>(_nextStates[0])) {
		// a->_probabilities[0] == I
		Distance* dists = (Distance*)(list);
		unsigned int RNG;
		float prob;

		for (int i = 0; i < dists->size(); i++) {
			// Calculating probability of Infected given distance
			prob = std::get<1>(_nextStates[0])->GetProb((*dists)[i]);

			RNG = rand() % 101;
			if (RNG > prob * 100) {
				ScheduleEventIn(std::get<2>(_nextStates[0])->GetRV(), StateMap::GetInstance()->GetAgentEventAction(std::get<0>(_nextStates[0]), _a)); // Infected Event
				return true; // Switched states 
			}
		}
		return false;
	}
	return true;
}

//---------------------Infected STATES-------------------------
void InfectedStateEvent::StateSpecificProcess()
{
	// Setting Schedule with transition function allowing Execute2() to schedule the next event
	_a->SetScheduled(StateInteractionProcess(nullptr));
}
// Application
bool SymptomStateEvent::StateInteractionProcess(Parameter* list)
{
	// _nextStates {string state name, float probability, distribution TimeDelay} 
	// Determining whether this is a terminating state
	unsigned int RNG = rand() % 101;
	float prob = ((Death_Age_HealthCondition_Prob*)std::get<1>(_nextStates[1]))->AgeGetProb(_a->GetAge()) + ((Death_Age_HealthCondition_Prob*)std::get<1>(_nextStates[1]))->HealthConditionGetProb(_a->GetPreExisting());
	if (RNG < (prob*100)) {
		ScheduleEventIn(std::get<2>(_nextStates[1])->GetRV(), StateMap::GetInstance()->GetAgentEventAction(std::get<0>(_nextStates[1]), _a));
		return true;
	}
	else {
		ScheduleEventIn(std::get<2>(_nextStates[0])->GetRV(), StateMap::GetInstance()->GetAgentEventAction(std::get<0>(_nextStates[0]), _a));
		return true;
	}
}

//---------------------OTHER STATES-------------------------
//Distribution* NonSusceptibleStateEvent::_timeDelay = nullptr;
void NonSusceptibleStateEvent::StateSpecificProcess() {
	// Setting Schedule with transition function allowing Execute2() to schedule
	_a->SetScheduled(StateInteractionProcess(nullptr));
}

bool RecoveredStateEvent::StateInteractionProcess(Parameter* list)
{
	STAT::GetInstance()->AddRecovery(_a->GetAge());
	return true;
}

bool DeadStateEvent::StateInteractionProcess(Parameter* list)
{
	STAT::GetInstance()->AddDeath(_a->GetAge());
	return true;
}
