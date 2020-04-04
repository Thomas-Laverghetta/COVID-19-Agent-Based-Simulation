#include "Environment.h"
#include <stdlib.h>

/*
	Will initialize state variables,
	create all agents in simulation, and
	schedule next move event
*/
Environment::Environment(unsigned int numAgents, unsigned int numInfected, unsigned int Ymax, unsigned int Xmax, unsigned int cellResolution, Time stepSize, DiseaseInfluence* DI)
{
	// initializing Disease Influence Variables 
	_envDI = DI;
	Agent::AgentState::SetDiseaseInfluence(_envDI);

	// Frequency of steps during simualation
	_stepSize = stepSize;

	// Setting domain
	_Ymax = Ymax;
	_Xmax = Xmax;
	_cellResolution = cellResolution;

	// initializing all the cells
	_cell = new CellNode**[1 + ((Ymax - 1) / cellResolution)];
	_cellCounter = new unsigned int* [1 + ((Ymax - 1) / cellResolution)];
	for (int i = 0; i < (1 + ((Ymax - 1) / cellResolution)); i++) {
		_cell[i] = new CellNode * [1 + ((Xmax - 1) / cellResolution)];
		_cellCounter = new unsigned int* [1 + ((Xmax - 1) / cellResolution)];
		for (int j = 0; j < (1 + ((Xmax - 1) / cellResolution)); j++)
			_cell[i][j] = nullptr;
	}

	// Creating all Agents Simulation
	Location coordinate;
	_agentRef = new Agent*[numAgents];
	for (int i = 0; i < numAgents-numInfected; i++) {
		coordinate._x = rand() % Xmax; coordinate._y = rand() % Ymax; // randoming choosing location with in domain
		_agentRef[i] = new Agent(coordinate, new HealthyEvent::HealthyState, rand() % 100 + 1);
	}

	// Infection Seed
	for (int i = 1; i < numInfected; i++) {
		coordinate._x = rand() % Xmax; coordinate._y= rand() % Ymax; 
		_agentRef[numAgents - numInfected + i] = new Agent(coordinate, new InfectionEvent::InfectedState, rand() % 100 + 1);
	}

	// Scheduling the first event
	ScheduleEventIn(_stepSize, new MoveEvent(this));
}


struct NodeTracker {
	Agent* _aRef;
	NodeTracker* _next;
	NodeTracker(Agent* a) {
		_aRef = a;
		_next = nullptr;
	}
};
class AgentTracker {
public:
	unsigned int _nodeCounter;

	NodeTracker* _head;

	void AddAgent(Agent* a) {
		NodeTracker* newNode = new NodeTracker(a);
		_nodeCounter++;
		newNode->_next = _head;
		_head = newNode;
	}
	
	~AgentTracker() {
		while (_head != nullptr) {
			NodeTracker* to_delete = _head;
			_head = _head->_next;
			delete to_delete;
			to_delete = nullptr;
		}
		_nodeCounter = 0;
	}

	NodeTracker* GetHead() {
		return _head;
	}
};


void Environment::MoveAgents()
{
	for (int r = 0; r < (1 + ((_Ymax - 1) / _cellResolution)); r++) {
		for (int c = 0; c < (1 + ((_Xmax - 1) / _cellResolution)); c++) {
			if (_cellCounter[r][c] > 1) {
				CellNode* curr = _cell[r][c];
				AgentTracker InfectedAgents;
				AgentTracker HealthyAgents;
				while (curr != nullptr) {
					if (curr->_aRef->_agentState->GetAgentSubState() == Healthy)
						HealthyAgents.AddAgent(curr->_aRef);
					else if (curr->_aRef->_agentState->GetAgentSubState() == Infected)
						InfectedAgents.AddAgent(curr->_aRef);

					curr = curr->_next;
				}
				if (HealthyAgents._nodeCounter > 0 && InfectedAgents._nodeCounter > 0) {
					float distance = 0;
					Parameter l;
					NodeTracker* curr_H = HealthyAgents.GetHead();
					NodeTracker* curr_I = InfectedAgents.GetHead();

					// Calculating the distance between healthy and infected
					while (curr_H != nullptr) {
						while (curr_I != nullptr) {
							// Calculating Distance
							distance = sqrtf((curr_H->_aRef->_location._x - curr_I->_aRef->_location._x) *
											(curr_H->_aRef->_location._x - curr_I->_aRef->_location._x) +
											(curr_H->_aRef->_location._y - curr_I->_aRef->_location._y) *
											(curr_H->_aRef->_location._y - curr_I->_aRef->_location._y))
											+ distance;
							curr_I = curr_I->_next;
						}
						l._distance = distance;
						curr_H->_aRef->_agentState->SetParameters(l);
						curr_H->_aRef->_agentState->StateTransition(curr_H->_aRef);
						curr_I = InfectedAgents.GetHead();
						curr_H = curr_H->_next;
						distance = 0;
					}
				}
			}
		}
	}
}

void Environment::AgentStatechange()
{

}
