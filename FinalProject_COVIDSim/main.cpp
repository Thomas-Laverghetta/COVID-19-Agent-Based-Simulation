#include "Environment.h"
#include "DerivEnv.h"

/*
	Basic How to use example:

	First - initialize StateMap by giving it the number of state to be used during simulation (S I R).

	Second - Register states and there coorisponding transition
*/

int main() {
	StateMap* instance = StateMap::GetInstance();
	instance->Initialize(3);

	/* 
	
		string is next state,
		float is probability (0 is terminating, 1 is state determination, else is probability specific), and 
		Distribution is time delay
	*/
	std::string SusceptibleState = "Susceptible"; std::tuple < std::string, Probability *, Distribution*> SusceptibleToInfected = std::make_tuple("Infected", new ConstantProbability(1), new Constant(0));
	instance->RegisterStateToNextState(SusceptibleState, &SusceptibleToInfected, 1);

	std::string InfectedState = "Infected"; std::tuple < std::string, Probability*, Distribution*> InfectedToNonSusceptible = std::make_tuple("NonSusceptible", new InfectionExponential(1.5), new Normal(13.2, 4.36));
	instance->RegisterStateToNextState(InfectedState, &InfectedToNonSusceptible, 1);

	std::string NonSusceptibleState = "NonSusceptible"; std::tuple < std::string, Probability*, Distribution*> NonSusceptibleTransition = std::make_tuple("", nullptr, nullptr);
	instance->RegisterStateToNextState(NonSusceptibleState, &NonSusceptibleTransition, 0);

	// Places in the events that corrispond to the same order associated states were placed
	Agent::AgentStateEventAction* StateEvents[] = { new SusceptibleStateEvent, new InfectedStateEvent, new NonSusceptibleStateEvent };
	instance->RegisterStatesToEvents(StateEvents);

	DerivEnv e("ENV",10, 10, 100, 400, 10, 1);
	
	RunSimulation(30.0f);
	return 0;
}