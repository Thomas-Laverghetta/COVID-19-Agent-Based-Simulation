#include "Environment.h"
#include "DerivEnv.h"
#include "HomeEnvironment.h"
#include <string>

/*
	Basic How to use example:

	First - initialize StateMap by giving it the number of state to be used during simulation (S I R).

	Second - Register states and there coorisponding transition
*/

// Everyone interacting with eachother everyday
void Application1() {
	//for (int i = 0; i < 100; i++) {
		DerivEnv e("Simulation", DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, 100, 1, 50, 50, 3, 1.0f);
		RunSimulation(100.0f);
	//}
}

// Quaritine 
void Application2() {
	//for (int i = 0; i < 100; i++) {
		DerivEnv e("WorldInteraction", DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, 10, 1, 50, 50, 3, 1.0f, DBG_NEW Constant(1));
		HomeEnvironment e2("HomeEnvironment", DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, 90, 0, DBG_NEW Triangular(2, 4, 6));


		Environment* WorldToHome[] = { &e2 };
		e.SetNextEnvironment(WorldToHome, DBG_NEW float(1.0f));

		Environment* HomeToWorld[] = { &e };
		e2.SetNextEnvironment(HomeToWorld, DBG_NEW float(1.0f));

		RunSimulation(100.0f);
	//}
}

int main() {
	StateMap* instance = StateMap::GetInstance();
	instance->Initialize(5);
	srand(time(NULL));
	/* 
		string is next state,
		float is probability (0 is terminating, 1 is state determination, else is probability specific), and 
		Distribution is time delay
	*/
	std::string SusceptibleState = "Susceptible"; std::tuple < std::string, Probability *, Distribution*> SusceptibleToExposed = std::make_tuple("Exposed", DBG_NEW InfectionExponential(0.25), DBG_NEW Constant(0));
	instance->RegisterStateToNextState(SusceptibleState, &SusceptibleToExposed, 1);

	std::string ExposedState = "Exposed"; std::tuple < std::string, Probability*, Distribution*> ExposedToInfected = std::make_tuple("Symptom", DBG_NEW ConstantProbability(1), DBG_NEW Triangular(2, 5.2, 14));
	instance->RegisterStateToNextState(ExposedState, &ExposedToInfected, 1);

	std::string SymptomState = "Symptom"; std::tuple < std::string, Probability*, Distribution*> SymptomToRecoveredORDead[] = { std::make_tuple("Recovered", DBG_NEW Recovered_Age_HealthCondition_Prob, DBG_NEW Normal(13.2, 4.36)), std::make_tuple("Dead", DBG_NEW Death_Age_HealthCondition_Prob, DBG_NEW Normal(13.2, 4.36)) };
	instance->RegisterStateToNextState(SymptomState, SymptomToRecoveredORDead, 2);

	std::string RecoveredState = "Recovered"; std::tuple < std::string, Probability*, Distribution*> RecoveredTransition = std::make_tuple("", nullptr, nullptr);
	instance->RegisterStateToNextState(RecoveredState, &RecoveredTransition, 0);

	std::string DeadState = "Dead"; std::tuple < std::string, Probability*, Distribution*> DeadTransition = std::make_tuple("", nullptr, nullptr);
	instance->RegisterStateToNextState(DeadState, &DeadTransition, 0);

	// Places in the events that corrispond to the same order associated states were placed
	Agent::AgentStateEventAction* StateEvents[] = { DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, DBG_NEW SymptomStateEvent, DBG_NEW RecoveredStateEvent, DBG_NEW DeadStateEvent  };
	instance->RegisterStatesToEvents(StateEvents);

	Application2();
	
	std::cout << "<termaination: press enter to exit> ";
	std::cin.get();
	
	_CrtDumpMemoryLeaks();
	return 0;
}