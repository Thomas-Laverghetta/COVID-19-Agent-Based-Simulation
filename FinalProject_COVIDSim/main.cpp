/*   __      ___
    /   \   /	 \   /    /
   /    /  /	 /  /    /
   \___/  /____ /   \___/

 Date: 04.01.2020
 Names and Main Tasking: 
 	Thomas Laverghetta - Project Lead, SINs Developer, and Simulation Framework
	Kyle Tanyag - Front End Developer (Application Developer)
	Cierra Hall - Researcher and Documentation Chief Editor
	Jayson Kreger - Visualization using Unity API and Collecting Statistics
	Bradon Waddell - Descrete-Event Simulation Executive
 
 Project:
 	To design and implement an agent-based simulator to simulate the effects of infections
	spreading through a population. 

 Inputs: 
	XML file containing agent states and their transitions. Read Parameter.xml to see
	intructions on use. 
 
 Outputs: 
	Output file containing agent's: identifier, locations (including specific environment), 
	high-lvl state, and low-lvl state. Another output file containing statistics for that
	simulation ran: sample#, #Suscetiple, #infected, #NonSusceptiple, and Infection Time.
	More statistics can be generated at the user's discretion.   

 Program Description: 
	

 For Questions:
	Please email Thomas Laverghetta (tlave002@odu.edu)
*///**********************************************************************************
#include "Environment.h"
#include "DerivEnv.h"
#include "HomeEnvironment.h"
#include <string>
#include "tinyxml2.h"
#include <stdio.h>
#include <string.h>
#include "SINs.h"

// Global Variables for Main
const char* parameterFile = "parameters.xml"; 						// File with states and their transitions
StateMap* stateInstance = StateMap::GetInstance(); 					// Instance to StateMap 
StateEventMap * stateEventInstance = StateEventMap::GetInstance();	// Instance to StateEventMap 
std::unordered_map<std::string, Variant*> VariantMap;				// 
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
			
			// Getting StateEvent and Register it to the curr list of event to use for sim
			stateInstance->RegisterStatesToEvents_IndividualEvent(stateEventInstance->GetStateEvent(stateName));
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
					for (int j = 0; j < numVariantParam; j++) {
						float temp;
						pElem->QueryFloatAttribute(std::string("varParam" + std::to_string(i) + std::to_string(j)).c_str(), &temp);
						distParam.AddParam(temp);
					}
					const char* VarName;
					pElem->QueryStringAttribute(std::string("variant" + std::to_string(i)).c_str(), &VarName);
					std::string VarNameStr = VarName;
					// Setting Time Delay Distribution
					std::get<2>(transitionRow[i]) = VariantMap.find(VarNameStr)->second->New(distParam);
				}

				// Setting Transition Row to StateMAP (state's name, its transition rows, and number of possible transitions)
				stateInstance->RegisterStateToNextState(stateName, transitionRow, numTransitions);
			}
			else {
				// Zero transition row
				std::tuple < std::string, Probability*, Variant*> transitionRow = std::make_tuple("", nullptr, nullptr);
				
				// Setting Transition Row to StateMAP (state's name, its transition rows, and number of possible transitions)
				stateInstance->RegisterStateToNextState(stateName, &transitionRow, 0);
			}
		}
		// Number of states user wants to use during simulation
		else if (elementName == "numStates")
		{
			int numStates;
			pElem->QueryAttribute("num", &numStates);
			
			// Initializing
			stateInstance->Initialize(numStates);
		}
	}

	return(true);
}

// Must Call this function first before anything else
void InitializeInfectionSimulation() {

	//// Implement using StateEventRegister for Variant and probability
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

	// Initializing Event Map
	if (!Load(parameterFile)){
		std::cout << "\a ERROR XML FILE NOT FOUND";
		exit(0);
	}
		
}

// Everyone interacting with eachother everyday
void Application1() {
	//for (int i = 0; i < 100; i++) {
		DerivEnv e("Simulation", DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, 100, 1, 20, 20, 6, 1.0f);
		RunSimulation(100.0f);
	//}
}

// Quarantining 
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
	
	// Initializing Random Seed for probabilities
	srand(time(NULL));

	// Application use for simulation
	Application1();

	// Termination of simulation
	std::cout << "<termaination: END OF SIM> press enter to exit: ";
	std::cin.get();
	
	// Memory Leak Checker
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

	// Places in the events that corrispond to the same order associated states were placed
	// Try to do in XML
	// Agent::AgentStateEventAction* StateEvents[] = { DBG_NEW SusceptibleStateEvent, DBG_NEW ExposedStateEvent, DBG_NEW SymptomStateEvent, DBG_NEW RecoveredStateEvent, DBG_NEW DeadStateEvent };
	// StateMap::GetInstance()->RegisterStatesToEvents_AllEvents(StateEvents);