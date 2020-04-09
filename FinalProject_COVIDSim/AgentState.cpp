#include "AgentState.h"

unsigned int STAT::_numInfected = 0;
unsigned int STAT::_numSusceptible = 0;
unsigned int STAT::_numOther = 0;

// Initializing Static Variables
unsigned int Agent::_nextId = 0;
Agent::Agent(Location& loc, EventAction * ea, unsigned int age)
{
	// Initializing variables
	_id = _nextId++;
	_location = loc;
	_age = age;
	
	ScheduleEventAt(0, ea);
	/*ea->Execute()*/; // setting the initial state of the agent
}


DiseaseInfluence* Agent::_DI = nullptr;

void Agent::StateTransition()
{
	if (!_scheduled) {
		// calling function to determine whether to change states and what state
		_scheduled = _stateTranitionFunction(this);
	}
}

std::string Agent::GetLowLevelState()
{
	return _lowLevelState;
}

SIR_States Agent::GetHighLevelState()
{
	return _highLevelState;
}


void Agent::SetHighLevelState(SIR_States subState)
{
	_highLevelState = subState;
}

void Agent::SetLowLevelState(std::string state)
{
	_lowLevelState = state;
}

void Agent::SetDiseaseInfluence(DiseaseInfluence* DI)
{
	_DI = DI;
}

void Agent::SetParameters(Parameter * list)
{
	//delete _list; // deleting previous dynamic object to point to new object
	_list = list;
}

//---------------------HEALTHY STATES-------------------------
float SusceptibleStateEvent::_expDistributionRate = 0.5;
void SusceptibleStateEvent::Execute()
{
	// Transitioning states
	_a->SetHighLevelState(Susceptible);
	_a->SetLowLevelState("Susceptible");
	_a->SetScheduled(false);
	/*STAT::_numSusceptible++; STAT::printSTAT();*/
	//printf("Agent %i | Low-level %s | High-level %i\n", _a->GetId(), _a->GetLowLevelState().c_str(), _a->GetHighLevelState());

	// Setting Agent Parameters for State
	delete _a->_probabilities; // deleting any dynamic memory associated with probabilities
	_a->_probabilities = new float;
	_a->_probabilities[0] = 1.0f; // initializing first value
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
void InfectedStateEvent::Execute()
{
	// Transitioning states
	_a->SetHighLevelState(Infected);
	_a->SetLowLevelState("Infected");
	//STAT::_numInfected++; STAT::printSTAT();
	//printf("Agent %i | Low-level %s | High-level %i\n", _a->GetId(), _a->GetLowLevelState().c_str(), _a->GetHighLevelState());

	// Setting Agent Parameters given state
	delete _a->_probabilities; // deleting any dynamic memory associated with probabilities
	_a->_probabilities = new float; // NEED TO CHANGE PENDING ON THE NUMBER OF STATES TO TRANSITION TOO
	_a->_probabilities[0] = 1.0f; // initializing first value

	_a->SetScheduled(StateTransition(_a));
}
bool InfectedStateEvent::StateTransition(Agent* a)
{
	SimulationExecutive::GetInstance()->ScheduleEventIn(_timeDelay->GetRV(), new RecoveredStateEvent(a)); // Recoved State
	return true;
}

//---------------------Recoved STATES-------------------------

void RecoveredStateEvent::Execute()
{
	// Transitioning states
	_a->SetHighLevelState(Other);
	_a->SetLowLevelState("Recovered");
	//STAT::_numOther++;	STAT::_numInfected--; STAT::printSTAT();
	//printf("Agent %i | Low-level %s | High-level %i\n", _a->GetId(), _a->GetLowLevelState().c_str(), _a->GetHighLevelState());

	// Setting Agent Parameters given state
	delete _a->_probabilities; // deleting any dynamic memory associated with probabilities
	_a->_probabilities = nullptr; // NEED TO CHANGE PENDING ON THE NUMBER OF STATES TO TRANSITION TOO
}
bool RecoveredStateEvent::StateTransition(Agent* a)
{
	return true;
}
