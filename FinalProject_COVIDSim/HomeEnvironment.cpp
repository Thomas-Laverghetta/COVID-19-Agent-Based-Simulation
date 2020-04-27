#include "HomeEnvironment.h"

HomeEnvironment::HomeEnvironment(std::string name, SusceptibleStateEvent* initialHealthyState, InfectedStateEvent* initialInfectedState, 
	unsigned int numSusceptible, unsigned int numInfected, Variant* agentInEnvDuration) 
	: Environment{ name, agentInEnvDuration, 1.0f, 0,  1, 1, numSusceptible, numInfected, initialHealthyState, initialInfectedState }
{}

void HomeEnvironment::EnvironmentProcess()
{}
