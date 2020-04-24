#pragma once
#include "Environment.h"
class HomeEnvironment : public Environment {
public:
	HomeEnvironment(std::string name = "HomeEnvironment", SusceptibleStateEvent* initialHealthyState = DBG_NEW SusceptibleStateEvent, InfectedStateEvent* initialInfectedState = DBG_NEW InfectedStateEvent, unsigned int numSusceptible = 100, unsigned int numInfected = 0
		, Variant* agentInEnvDuration = DBG_NEW TriangularRV(4,5,9));
private:
	void EnvironmentProcess();
};

