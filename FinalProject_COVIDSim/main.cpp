#include "Environment.h"
#include "DerivEnv.h"
#include "HomeEnvironment.h"
#include <string>
#include "tinyxml2.h"
#include <stdio.h>
#include <string.h>
#include <unordered_map>

const char* parameterFile = "parameters.xml";
StateMap* instance = StateMap::GetInstance();
std::unordered_map<std::string, Variant*> VariantMap;
std::unordered_map<std::string, Probability*> probabilityMap;

// Loading from XML file
bool Load(const char* filename)
{
	std::cout << "Current Working Directory =" << filename << std::endl;

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError result = doc.LoadFile(filename);

	if (result != tinyxml2::XML_SUCCESS)
		return(false);

	tinyxml2::XMLElement* pRoot = doc.RootElement();

	for (tinyxml2::XMLElement* pElem = pRoot->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string elementName = pElem->Value();

		if (elementName == "state")
		{
			int numTransitions;
			const char* stateName;
			pElem->QueryStringAttribute("name", &stateName);
			std::cout << stateName << std::endl;

			// Number of transition that can occur
			pElem->QueryAttribute("numTransitions", &numTransitions);
			
			if (numTransitions > 0) {
				// allocating number of transitions needed
				std::tuple < std::string, Probability*, Variant*> * transitionRow = new std::tuple < std::string, Probability*, Variant*>[numTransitions];

				// iterates through each transition row
				for (int i = 0; i < numTransitions; i++) {
					// Next State Name
					const char* nextState;
					pElem->QueryStringAttribute(std::string("nextState" + std::to_string(i)).c_str(), &nextState);
					std::get<0>(transitionRow[i]) = nextState;
					std::cout << std::get<0>(transitionRow[i]) << std::endl;

					// Probability distribution
					int numProbParam;
					pElem->QueryAttribute(std::string("numProbParam" + std::to_string(i)).c_str(), &numProbParam);
					ProbabilityParameters param(numProbParam);
					for (int j = 0; j < numProbParam; j++) {
						float temp;
						pElem->QueryFloatAttribute(std::string("probParam" + std::to_string(i) + std::to_string(j)).c_str(), &temp);
						param.AddParam(temp);
					}
					const char * ProbName;
					pElem->QueryStringAttribute(std::string("probability" + std::to_string(i)).c_str(), &ProbName);
					std::string ProbNamestr = ProbName;
					// Setting Probability Distribution
					std::get<1>(transitionRow[i]) = probabilityMap.find(ProbNamestr)->second->New(param);


					// Setting Time delay distribution
					int numVariantParam;
					pElem->QueryAttribute(std::string("numVariantParam" + std::to_string(i)).c_str(), &numVariantParam);
					ProbabilityParameters distParam(numVariantParam);
					for (int j = 0; j < numProbParam; j++) {
						float temp;
						pElem->QueryFloatAttribute(std::string("varParam" + std::to_string(i) + std::to_string(j)).c_str(), &temp);
						distParam.AddParam(temp);
					}
					const char* VarName;
					pElem->QueryStringAttribute(std::string("variant" + std::to_string(i)).c_str(), &VarName);
					std::string VarNameStr = VarName;
					// Setting Time Delay Distribution
					std::get<2>(transitionRow[i]) = VariantMap.find(VarNameStr)->second->New(param);
				}

				// Setting Transition Row to StateMAP
				instance->RegisterStateToNextState(stateName, transitionRow, numTransitions);
			}
			else {
				// Zero transition row
				std::tuple < std::string, Probability*, Variant*> transitionRow = std::make_tuple("", nullptr, nullptr);
				instance->RegisterStateToNextState(stateName, &transitionRow, 0);
			}
		}
		else if (elementName == "numStates")
		{
			int numStates;
			pElem->QueryAttribute("num", &numStates);
			
			// Initializing
			StateMap::GetInstance()->Initialize(numStates);
		}
	}

	return(true);
}

// Must Call this function first before anything else
void InitializeInfectionSimulation() {
	// Imitializing Probability Map (used for probability between transitions)
	probabilityMap = { 
		{"Constant", new ConstantProb(0)},
		{"Distance", new DistanceProb(0)},
		{"Recover", new Recovered_Age_HealthCondition_Prob},
		{"Death", new Death_Age_HealthCondition_Prob}
	};

	// Initializing Variant Map
	VariantMap = {
		{"Exponential", new ExponentialRV(0)},
		{"Uniform", new UniformRV(0,0)},
		{"Triangular",new TriangularRV(0,0,0)},
		{"Normal",new NormalRV(0,0)},
		{"PoissonRV",new PoissonRV(0)},
		{"Constant", new ConstantRV(0)},
		{"Weibell", new WeibullRV(0,0)},
		{"Erlang", new ErlangRV(0,0)}
	};


	Load(parameterFile);


}

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
		DerivEnv e("WorldInteraction", DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, 10, 1, 50, 50, 3, 1.0f, DBG_NEW ConstantRV(1));
		HomeEnvironment e2("HomeEnvironment", DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, 90, 0, DBG_NEW TriangularRV(2, 4, 6));


		Environment* WorldToHome[] = { &e2 };
		e.SetNextEnvironment(WorldToHome, DBG_NEW float(1.0f));

		Environment* HomeToWorld[] = { &e };
		e2.SetNextEnvironment(HomeToWorld, DBG_NEW float(1.0f));

		RunSimulation(100.0f);
	//}
}

int main() {
	// Initializes simulation parameters for infection sim
	InitializeInfectionSimulation();
	srand(time(NULL));

	// Places in the events that corrispond to the same order associated states were placed
	// Try to do in XML
	Agent::AgentStateEventAction* StateEvents[] = { DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, DBG_NEW SymptomStateEvent, DBG_NEW RecoveredStateEvent, DBG_NEW DeadStateEvent };
	StateMap::GetInstance()->RegisterStatesToEvents(StateEvents);
	Application1();
	std::cout << "<termaination: press enter to exit> ";
	std::cin.get();
	
	_CrtDumpMemoryLeaks();
	return 0;
}



/*
	string is next state,
	float is probability (0 is terminating, 1 is state determination, else is probability specific), and
	Variant is time delay


	Next Step would be to make this into a XML file format instead
*/
//std::string SusceptibleState = "Susceptible"; std::tuple < std::string, Probability *, Variant*> SusceptibleToExposed = std::make_tuple("Exposed", DBG_NEW DistanceProbability(0.25), DBG_NEW Constant(0));
//instance->RegisterStateToNextState(SusceptibleState, &SusceptibleToExposed, 1);

//std::string ExposedState = "Exposed"; std::tuple < std::string, Probability*, Variant*> ExposedToInfected = std::make_tuple("Symptom", DBG_NEW ConstantProbability(1), DBG_NEW Triangular(2, 5.2, 14));
//instance->RegisterStateToNextState(ExposedState, &ExposedToInfected, 1);

//std::string SymptomState = "Symptom"; std::tuple < std::string, Probability*, Variant*> SymptomToRecoveredORDead[] = { std::make_tuple("Recovered", DBG_NEW Recovered_Age_HealthCondition_Prob, DBG_NEW Normal(13.2, 4.36)), std::make_tuple("Dead", DBG_NEW Death_Age_HealthCondition_Prob, DBG_NEW Normal(13.2, 4.36)) };
//instance->RegisterStateToNextState(SymptomState, SymptomToRecoveredORDead, 2);

//std::string RecoveredState = "Recovered"; std::tuple < std::string, Probability*, Variant*> RecoveredTransition = std::make_tuple("", nullptr, nullptr);
//instance->RegisterStateToNextState(RecoveredState, &RecoveredTransition, 0);

//std::string DeadState = "Dead"; std::tuple < std::string, Probability*, Variant*> DeadTransition = std::make_tuple("", nullptr, nullptr);
//instance->RegisterStateToNextState(DeadState, &DeadTransition, 0);

