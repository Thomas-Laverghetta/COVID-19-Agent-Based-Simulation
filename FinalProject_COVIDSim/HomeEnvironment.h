#pragma once
#include "Environment.h"
class HomeEnvironment : public Environment {
public:
	HomeEnvironment(std::string name = "HomeEnvironment", SusceptibleStateEvent* initialHealthyState = DBG_NEW SusceptibleStateEvent, InfectedStateEvent* initialInfectedState = DBG_NEW InfectedStateEvent, unsigned int numSusceptible = 100, unsigned int numInfected = 0
		, Distribution* agentInEnvDuration = DBG_NEW Triangular(4,5,9));
private:
	bool EnvironmentProcess();

};

