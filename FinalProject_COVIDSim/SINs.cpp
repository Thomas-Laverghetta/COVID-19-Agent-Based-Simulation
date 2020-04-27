#include "SINs.h"

// Registeration SINs
bool SusceptibleStateEvent::SusceptibleRegister = StateEventMap::GetInstance()->RegisterStateEvent(new SusceptibleStateEvent);
bool InfectedStateEvent::InfectedRegister = StateEventMap::GetInstance()->RegisterStateEvent(new InfectedStateEvent);
bool NonSusceptibleStateEvent::NonSusceptibleRegister = StateEventMap::GetInstance()->RegisterStateEvent(new NonSusceptibleStateEvent);

// Registeration Application
bool ExposedStateEvent::ExposedRegister = StateEventMap::GetInstance()->RegisterStateEvent(new ExposedStateEvent);
bool SymptomStateEvent::SymptomRegister = StateEventMap::GetInstance()->RegisterStateEvent(new SymptomStateEvent);
bool RecoveredStateEvent::RecoveredRegister = StateEventMap::GetInstance()->RegisterStateEvent(new RecoveredStateEvent);
bool DeadStateEvent::DeadRegister = StateEventMap::GetInstance()->RegisterStateEvent(new DeadStateEvent);



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
	// _nextStates {string state name, float probability, Variant TimeDelay} 
	// Determining whether this is a terminating state
	unsigned int RNG = rand() % 101;
	float prob = ((Death_Age_HealthCondition_Prob*)std::get<1>(_nextStates[1]))->AgeGetProb(_a->GetAge()) + ((Death_Age_HealthCondition_Prob*)std::get<1>(_nextStates[1]))->HealthConditionGetProb(_a->GetPreExisting());
	if (RNG < (prob * 100)) {
		ScheduleEventIn(std::get<2>(_nextStates[1])->GetRV(), StateMap::GetInstance()->GetAgentEventAction(std::get<0>(_nextStates[1]), _a));
		return true;
	}
	else {
		ScheduleEventIn(std::get<2>(_nextStates[0])->GetRV(), StateMap::GetInstance()->GetAgentEventAction(std::get<0>(_nextStates[0]), _a));
		return true;
	}
}

//---------------------OTHER STATES-------------------------
//Variant* NonSusceptibleStateEvent::_timeDelay = nullptr;
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