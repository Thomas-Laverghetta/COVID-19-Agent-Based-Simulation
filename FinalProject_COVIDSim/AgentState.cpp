#include "AgentState.h"
//---------------STAT----------------
unsigned int STAT::_numInfected = 0;
unsigned int STAT::_numSusceptible = 0;
unsigned int STAT::_numOther = 0;

// Initializing Static Variables
unsigned int Agent::_nextId = 0;
DiseaseInfluence* Agent::_dI = nullptr;

StateMap* StateMap::_instance = nullptr;

Agent::Agent(Location& loc, AgentEventAction* aea, unsigned int age)
{
	// Initializing variables
	_id = _nextId++;
	_location = loc;
	_age = age;
	_aea = aea;
	
	// For Statistics
	_aea->InitialSetHighLevelState();

	// Setting the event to this agent
	_aea->SetAgent(this);

	// setting the initial state of the agent: High and Low states
	ScheduleEventAt(0, aea); 
}

// Agent determines whether to transition based on interactions or time
void Agent::StateTransition()
{
	// If transition has been scheduled
	if (!_scheduled) {
		// calling function to determine whether to change states and what state
		_scheduled = _aea->StateTransitionProcess();
	}
}

void Agent::SetParameters(Parameter* list)
{
	_aea->SetParameterList(list);
}

void Agent::SetAgentEventAction(AgentEventAction* aea)
{
	_aea = aea;
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
void Agent::AgentEventAction::Execute()
{
	// Transitioning states
	_a->SetHighLevelState(_highLevelState);
	_a->SetLowLevelState(_lowLevelState);
	_a->SetScheduled(false); // RESETTING SCH IN AGENT
	_a->SetAgentEventAction(this);

	// Register to get next states
	unsigned int i = 0;
	_nextStates = StateMap::GetInstance()->GetNextStates(_lowLevelState, _numNextStates, i);
	_nextProbabilities = StateMap::GetInstance()->GetProbabilities(i);


	// Specific State Event Execution
	Execute2();
}

void Agent::AgentEventAction::InitialSetHighLevelState() {
	// Prelude Stat
	if (_highLevelState == Susceptible)
		STAT::_numSusceptible++;
	else if (_highLevelState == Infected)
		STAT::_numInfected++;
	else
		STAT::_numOther++;
}

void Agent::AgentEventAction::SetHighLevelState(SIR_States subState)
{
	// Prelude Stat
	if (_highLevelState == Susceptible)
		STAT::_numSusceptible--;
	else if (_highLevelState == Infected)
		STAT::_numInfected--;
	else
		STAT::_numOther--;

	_highLevelState = subState;

	// Epilogue Stat
	if (_highLevelState == Susceptible)
		STAT::_numSusceptible++;
	else if (_highLevelState == Infected)
		STAT::_numInfected++;
	else
		STAT::_numOther++;
}

//---------------------HEALTHY STATES-------------------------
float SusceptibleStateEvent::_expDistributionRate = 0.5;
void SusceptibleStateEvent::Execute2()
{
	//// Setting State Transition function
	//_a->_stateTranitionFunction = SusceptibleStateEvent::StateTransition;
}
bool SusceptibleStateEvent::StateTransitionProcess()
{
	// a->_probabilities[0] == I
	Distance* dists = (Distance *)(_list);
	float H = _nextProbabilities[0] != 1.0f ? 1- _nextProbabilities[0] : 1.0f;
	float I_prob = 0.0f;
	unsigned int RNG;
	float tempProb_I;
	for (int i = 0; i < dists->size(); i++) {
		// Calculating probability of Infected given distance
		I_prob = exp(-_expDistributionRate * (*dists)[i]) - exp(-FLT_MAX) - 0.05;

		// Calculating non normalized probability
		tempProb_I = _nextProbabilities[0] * I_prob;
		H = H * (1 - I_prob);

		// Normalizing
		tempProb_I = tempProb_I / (tempProb_I + H);
		H = H / (tempProb_I + H);

		if (tempProb_I > _nextProbabilities[0])
			_nextProbabilities[0] = tempProb_I;

		RNG = rand() % 101;
		if (RNG > H*100) {
			ScheduleEventIn(_nextStates[0].second->GetRV(), StateMap::GetInstance()->GetAgentEventAction(_nextStates[0].first)->New(_a)); // Infected Event
			return true; // Switched states 
		}
	}
	return false;
}
//void SusceptibleStateEvent::SetNextEnvironment(AgentEventAction** nextStates, float* nextStateProbabilities)
//{
//	_nextStates = nextStates;
//	_nextStateProbabilities = nextStateProbabilities;
//}

//---------------------Infected STATES-------------------------
//Distribution* InfectedStateEvent::_timeDelay = new Triangular(5, 15, 25);
void InfectedStateEvent::Execute2()
{
	// Setting Schedule with transition function allowing Execute2() to schedule the next event
	_a->SetScheduled(StateTransitionProcess());
}
bool InfectedStateEvent::StateTransitionProcess()
{
	// _nextStates {string state name, float probability, distribution TimeDelay} 

	// Determining whether this is a terminating state
	if (_nextProbabilities[0] > 0) {
		unsigned int RNG = rand() % 101;
		unsigned int i = -1;
		float temp = 0;
		do {
			i++;
			temp = _nextProbabilities[i] + temp;
		} while (temp*100 < RNG);
		ScheduleEventIn(_nextStates[i].second->GetRV(), StateMap::GetInstance()->GetAgentEventAction(_nextStates[i].first)->New(_a));
		return true;
	}
	return true;
}

//---------------------OTHER STATES-------------------------
//Distribution* NonSusceptibleStateEvent::_timeDelay = nullptr;
void NonSusceptibleStateEvent::Execute2() { 
	// Setting Schedule with transition function allowing Execute2() to schedule
	_a->SetScheduled(StateTransitionProcess());
}
bool NonSusceptibleStateEvent::StateTransitionProcess() {
	// _nextStates {string state name, float probability, distribution TimeDelay} 

	// Determining whether this is a terminating state
	if (_nextProbabilities[0] > 0) {
		unsigned int RNG = rand() % 101;
		unsigned int i = -1;
		float temp = 0;
		do {
			i++;
			temp = _nextProbabilities[i] + temp;
		} while (temp < RNG);
		ScheduleEventIn(_nextStates[i].second->GetRV(), StateMap::GetInstance()->GetAgentEventAction(_nextStates[i].first)->New(_a));
		return true;
	}
	return true;
}

