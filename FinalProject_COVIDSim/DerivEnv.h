#ifndef DERIVENV_H
#define DERIVENV_H
#include "Environment.h"
class DerivEnv : public Environment
{
public:
	/*
		Creates Agents and Sets ranges
	*/
	DerivEnv(std::string name = "DerivEnvironment", unsigned int numSusceptible = 100, unsigned int numInfected = 3, unsigned int Ymax = 100, unsigned int Xmax = 100,
		unsigned int cellResolution = 10, Time moveFrequency = 1.0f, Distribution * agentInEnvDuration = nullptr);
private:
	bool EnvironmentProcess();
	void CheckAgentDistances();
};
#endif


