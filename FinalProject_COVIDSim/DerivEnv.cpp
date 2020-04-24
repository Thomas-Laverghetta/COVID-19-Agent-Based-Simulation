#include "DerivEnv.h"
/*
	Will initialize state variables,
	create all agents in simulation, and
	schedule next move event
*/
DerivEnv::DerivEnv(std::string name, SusceptibleStateEvent * initialHealthyState, InfectedStateEvent * initialInfectedState, unsigned int numSusceptible, unsigned int numInfected, unsigned int Ymax, unsigned int Xmax,
	unsigned int cellResolution, Time moveFrequency, Variant* agentInEnvDuration)
	: Environment{ name, agentInEnvDuration, moveFrequency, cellResolution,  Ymax, Xmax, numSusceptible, numInfected, initialHealthyState, initialInfectedState }
{}

void DerivEnv::EnvironmentProcess()
{
	// Moving Agnets
	MoveAgents();
	CheckAgentDistances();
}