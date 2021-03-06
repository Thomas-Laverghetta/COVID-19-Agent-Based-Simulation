#ifndef DERIVENV_H
#define DERIVENV_H
#include "Environment.h"
class DerivEnv : public Environment
{
public:
	/*
		Creates Agents and Sets ranges
	*/
	DerivEnv(std::string name = "DerivEnvironment", SusceptibleStateEvent* initialHealthyState = DBG_NEW SusceptibleStateEvent, InfectedStateEvent* initialInfectedState = DBG_NEW InfectedStateEvent, unsigned int numSusceptible = 100, unsigned int numInfected = 3, unsigned int Ymax = 100, unsigned int Xmax = 100,
		unsigned int cellResolution = 10, Time moveFrequency = 1.0f, Variant * agentInEnvDuration = nullptr);
private:
	void EnvironmentProcess();
};
#endif


