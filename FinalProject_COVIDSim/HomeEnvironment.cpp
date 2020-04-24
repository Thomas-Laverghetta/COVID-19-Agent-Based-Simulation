#include "HomeEnvironment.h"

HomeEnvironment::HomeEnvironment(std::string name, SusceptibleStateEvent* initialHealthyState, InfectedStateEvent* initialInfectedState, 
	unsigned int numSusceptible, unsigned int numInfected, Variant* agentInEnvDuration) 
	: Environment{ name, agentInEnvDuration, 1.0f, 0,  1, 1, numSusceptible, numInfected, initialHealthyState, initialInfectedState }
{
	//// Display contents home environment when agent switch environments
	//_agentList.SetAddRemoveStat(true);
}

void HomeEnvironment::EnvironmentProcess()
{
	_agentList.PrintAgents();
}
