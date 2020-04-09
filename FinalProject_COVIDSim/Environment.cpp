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
	Agent::SetDiseaseInfluence(_envDI);

	// Frequency of steps during simualation
	_stepSize = stepSize;

	// Setting domain
	_Ymax = Ymax;
	_Xmax = Xmax;
	_cellResolution = cellResolution;

	// Initializing and Creating cells
	_cellContainer.CreateCells();

	// Creating all Agents Simulation
	Location coordinate;
	_agentRef = new Agent*[numAgents];
	for (int i = 0; i < numAgents-numInfected; i++) {
		coordinate._x = rand() % Xmax; coordinate._y = rand() % Ymax; // randoming choosing location with in domain
		SusceptibleStateEvent* ea = new SusceptibleStateEvent;
		_agentRef[i] = new Agent(coordinate, ea, rand() % 100 + 1);
		_cellContainer.AddAgent(_agentRef[i]);
		ea->SetAgent(_agentRef[i]);
	}

	// Infection Seed
	for (int i = numAgents - numInfected; i < numAgents; i++) {
		coordinate._x = rand() % Xmax; coordinate._y= rand() % Ymax; 
		InfectedStateEvent* ise = new InfectedStateEvent;
		_agentRef[i] = new Agent(coordinate, ise, rand() % 100 + 1);
		_cellContainer.AddAgent(_agentRef[i]);
		ise->SetAgent(_agentRef[i]);
	}

	_numAgents = numAgents;
	// Scheduling the first event
	ScheduleEventIn(_stepSize, new MoveEvent(this));

	/* initialize random seed: */
	//srand(time(NULL));
}

class AgentTracker {
public:
	unsigned int _nodeCounter;
	
	struct NodeTracker {
		Agent* _aRef;
		NodeTracker* _next;
		NodeTracker(Agent* a) {
			_aRef = a;
			_next = nullptr;
		}
	};
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

void Environment::CheckAgentDistances()
{
	if (STAT::_numSusceptible > 0) {
		unsigned int coln_max = (1 + ((_Xmax - 1) / _cellResolution));
		unsigned int row_max = (1 + ((_Xmax - 1) / _cellResolution));
		for (int r = 0; r < (1 + ((_Ymax - 1) / _cellResolution)); r++) {
			for (int c = 0; c < (1 + ((_Xmax - 1) / _cellResolution)); c++) {
				if (_cellContainer.GetCounterValue(r, c) > 1) {
					AgentTracker InfectedAgents;
					AgentTracker HealthyAgents;
					CellLinkedList::CellNode* curr = _cellContainer.GetCellRef(r, c);
					bool doLoop = true;	// to state whether to change curr cell
					//bool loop = true;	// to state whether or not to interate through cell 
					short int i = 0;	// counter of number of do-while loops 
					bool orginialCell = true; // Too not search for infected agents when outside orginial cell
					do {
						// Finding Infected and Healthy Agents and saving reference
						while (curr != nullptr /*&& loop*/) {
							if (curr->_aRef->GetHighLevelState() == Susceptible)
								HealthyAgents.AddAgent(curr->_aRef);
							else if (orginialCell ? curr->_aRef->GetHighLevelState() == Infected : false)
								InfectedAgents.AddAgent(curr->_aRef);

							curr = curr->_next;
						}

						// Determine if current cell has no infected to infect other cells
						if (orginialCell ? InfectedAgents._nodeCounter == 0 : false) {
							i = -1; // Dont search other cells around
						}
						//loop = false; // until indicated, dont loop through cell

						// Searching 1 cell out to determine affects of other agents on currents and vis versa
						switch (i) {
						case 0:
							orginialCell = false; // orginial cell has been searched, dont search for infected in other cells relative to this cell

							// right
							if (c + 1 < coln_max ? _cellContainer.GetCounterValue(r, c + 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r, c + 1);
							}
							break;
						case 1:
							// left
							if (c - 1 >= 0 ? _cellContainer.GetCounterValue(r, c - 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r, c - 1);
							}
							break;
						case 2:
							// top
							if (r + 1 < row_max ? _cellContainer.GetCounterValue(r + 1, c) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r + 1, c);
							}
							break;
						case 3:
							// top-right
							if (c + 1 < coln_max && r + 1 < row_max ? _cellContainer.GetCounterValue(r + 1, c + 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r + 1, c + 1);
							}
							break;
						case 4:
							// top-left
							if (c - 1 >= 0 && r + 1 < row_max ? _cellContainer.GetCounterValue(r + 1, c - 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r + 1, c - 1);
							}
							break;
						case 5:
							// bottom
							if (r - 1 >= 0 ? _cellContainer.GetCounterValue(r - 1, c) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r - 1, c);
							}
							break;
						case 6:
							// bottom-right
							if (c + 1 < coln_max && r - 1 >= 0 ? _cellContainer.GetCounterValue(r - 1, c + 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r - 1, c + 1);
							}
							break;
						case 7:
							// bottom-left
							if (c - 1 >= 0 && r - 1 >= 0 ? _cellContainer.GetCounterValue(r - 1, c - 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellRef(r - 1, c - 1);
							}
							break;
						default:
							doLoop = false;
							break;
						}
						i++; // next test
					} while (doLoop);
					// If there is infected and healthy agents
					if (HealthyAgents._nodeCounter > 0 && InfectedAgents._nodeCounter > 0) {
						Distance distance(InfectedAgents._nodeCounter);
						AgentTracker::NodeTracker* curr_H = HealthyAgents.GetHead();
						AgentTracker::NodeTracker* curr_I = InfectedAgents.GetHead();

						// Calculating the distance between healthy and infected
						while (curr_H != nullptr) {
							while (curr_I != nullptr) {
								// Calculating Distance
								distance.AddDistance(sqrtf((curr_H->_aRef->GetLocation()._x - curr_I->_aRef->GetLocation()._x) *
									(curr_H->_aRef->GetLocation()._x - curr_I->_aRef->GetLocation()._x) +
									(curr_H->_aRef->GetLocation()._y - curr_I->_aRef->GetLocation()._y) *
									(curr_H->_aRef->GetLocation()._y - curr_I->_aRef->GetLocation()._y)),
									curr_I->_aRef->GetLowLevelState());
								curr_I = curr_I->_next;
							}
							curr_H->_aRef->SetParameters(&distance);
							curr_H->_aRef->StateTransition();
							curr_I = InfectedAgents.GetHead();
							curr_H = curr_H->_next;
							distance.resetIndex(); // resets index on float[] for next Healthy agent
						}
						// PLOBLEM
					}
				}
			}
		}
	}
}

// Random Movements
void Environment::MoveAgents()
{
	_cellContainer.~CellLinkedList();
	Location coordinate;
	unsigned int numCells_y = 1 + ((_Ymax - 1) / _cellResolution);
	unsigned int numCells_x = 1 + ((_Xmax - 1) / _cellResolution);
	for (int i = 0; i < _numAgents; i++) {
		// Calculating random movement
		coordinate._x = abs((int)(rand() % _Xmax + _agentRef[i]->GetLocation()._x)) % _Xmax * _stepSize;
		coordinate._y = abs((int)(rand() % _Ymax + _agentRef[i]->GetLocation()._y)) % _Ymax * _stepSize;
		_agentRef[i]->SetLocation(coordinate);
		_cellContainer.AddAgent(_agentRef[i]);
		
	}
}
