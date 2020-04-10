#include "AgentState.h"

// Initializing Static Variables
unsigned int Agent::_nextId = 0;
DiseaseInfluence* Agent::_DI = nullptr;

Agent::Agent(Location& loc, EventAction * ea, unsigned int age)
{
	// Initializing variables
	_id = _nextId++;
	_location = loc;
	_age = age;
	
	// setting the initial state of the agent: High and Low states
	ScheduleEventAt(0, ea); 
}

void Agent::StateTransition()
{
	// If transition has been scheduled
	if (!_scheduled) {
		// calling function to determine whether to change states and what state
		_scheduled = _stateTranitionFunction(this);
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
void AgentEventAction::Execute()
{
	// Transitioning states
	_a->SetHighLevelState(_highLevelState);
	_a->SetLowLevelState(_lowLevelState);
	_a->SetScheduled(false); // RESETTING SCH IN AGENT

	// Setting Agent Parameters for State
	delete _a->_probabilities; // deleting any dynamic memory associated with probabilities
	if (_numProbabilities > 0) {
		_a->_probabilities = new float[_numProbabilities];
		for (unsigned int i = 0; i < _numProbabilities; i++)
			_a->_probabilities[i] = 1.0f; // initializing probabilities value
	}
	else
		_a->_probabilities = nullptr;

	// Specific State Event Execution
	Execute2();
}

//---------------------HEALTHY STATES-------------------------
float SusceptibleStateEvent::_expDistributionRate = 0.5;
void SusceptibleStateEvent::Execute2()
{
	// Setting State Transition function
	_a->_stateTranitionFunction = SusceptibleStateEvent::StateTransition;
}
bool SusceptibleStateEvent::StateTransition(Agent* a)
{
	// a->_probabilities[0] == I
	Distance* dists = (Distance *)(a->_list);
	float H = a->_probabilities[0] != 1.0f ? 1-a->_probabilities[0] : 1.0f;
	float I_prob = 0.0f;
	unsigned int RNG;
	for (int i = 0; i < dists->size(); i++) {
		// Calculating probability of Infected given distance
		I_prob = exp(-_expDistributionRate * (*dists)[i]) - exp(-FLT_MAX) - 0.05;

		// Calculating non normalized probability
		a->_probabilities[0] = a->_probabilities[0] * I_prob;
		H = H * (1 - I_prob);

		// Normalizing
		a->_probabilities[0] = a->_probabilities[0] / (a->_probabilities[0] + H);
		H = H / (a->_probabilities[0] + H);

		RNG = rand() % 101;
		if (RNG > H*100) {
			SimulationExecutive::GetInstance()->ScheduleEventIn(0, new InfectedStateEvent(a)); // Infected Event
			return true; // Switched states 
		}
	}
	return false;
}

//---------------------Infected STATES-------------------------
Distribution* InfectedStateEvent::_timeDelay = new Triangular(5, 10, 25);
void InfectedStateEvent::Execute2()
{
	// Setting State Transition function
	_a->_stateTranitionFunction = InfectedStateEvent::StateTransition;

	// Setting Schedule with transition function allowing Execute2() to schedule the next event
	_a->SetScheduled(StateTransition(_a));
}
bool InfectedStateEvent::StateTransition(Agent* a)
{
	SimulationExecutive::GetInstance()->ScheduleEventIn(_timeDelay->GetRV(), new NonSusceptibleStateEvent(a));
	return true;
}

//---------------------Recoved STATES-------------------------
void NonSusceptibleStateEvent::Execute2() { 
	// Setting State Transition function
	_a->_stateTranitionFunction = NonSusceptibleStateEvent::StateTransition;

	// Setting Schedule with transition function allowing Execute2() to schedule
	_a->SetScheduled(StateTransition(_a));
}
bool NonSusceptibleStateEvent::StateTransition(Agent* a) { return true; }
