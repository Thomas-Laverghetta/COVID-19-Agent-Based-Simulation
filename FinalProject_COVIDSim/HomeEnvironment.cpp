#include "HomeEnvironment.h"

HomeEnvironment::HomeEnvironment(std::string name, SusceptibleStateEvent* initialHealthyState, InfectedStateEvent* initialInfectedState, 
	unsigned int numSusceptible, unsigned int numInfected, Distribution* agentInEnvDuration) 
	: Environment{ name, agentInEnvDuration, 0, 0,  1, 1, numSusceptible, numInfected, initialHealthyState, initialInfectedState }
{}

bool HomeEnvironment::EnvironmentProcess()
{
	return true;
}
