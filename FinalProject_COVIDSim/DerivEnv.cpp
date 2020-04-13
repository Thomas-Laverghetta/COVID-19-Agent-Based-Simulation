#include "DerivEnv.h"
/*
	Will initialize state variables,
	create all agents in simulation, and
	schedule next move event
*/
DerivEnv::DerivEnv(unsigned int numSusceptible, unsigned int numInfected, unsigned int Ymax, unsigned int Xmax,
	unsigned int cellResolution, Time moveFrequency, std::string name, Distribution* agentInEnvDuration) 
	: Environment{ name, agentInEnvDuration, moveFrequency }
{
	// initializing Disease Influence Variables 
	_envDI = nullptr; ///////////////////////////////////////
	Agent::SetDiseaseInfluence(_envDI);

	// Frequency of steps during simualation
	_moveFrequency = moveFrequency;

	// Setting domain
	_domain._x = Xmax;
	_domain._y = Ymax;
	_cellResolution = cellResolution;

	// Initializing and Creating cells
	_cellContainer.CreateCells();

	// Creating all Agents Simulation
	_agentList.CreateAgents(numSusceptible, numInfected, new SusceptibleStateEvent, new InfectedStateEvent, this);
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

void DerivEnv::CheckAgentDistances()
{
	if (STAT::_numSusceptible > 0) {
		unsigned int coln_max = (1 + ((_domain._x - 1) / _cellResolution));
		unsigned int row_max = (1 + ((_domain._y - 1) / _cellResolution));
		for (int r = 0; r < row_max; r++) {
			for (int c = 0; c < coln_max; c++) {
				if (_cellContainer.GetCounterValue(r, c) > 0) {
					AgentTracker InfectedAgents; // Containers to hold found infected and healthy agents
					AgentTracker HealthyAgents;
					CellLinkedList::CellNode* curr = _cellContainer.GetCellHead(r, c); // refer to head of cell list
					bool doLoop = true;			// to state whether to change curr cell
					short int i = 0;			// counter of number of do-while loops 
					bool orginialCell = true; 	// Too not search for infected agents when outside orginial cell
					do {
						// Finding Infected and Healthy Agents and saving reference
						while (curr != nullptr) {
							if (curr->_aRef->GetHighLevelState() == Susceptible)
								HealthyAgents.AddAgent(curr->_aRef);
							else if (orginialCell && curr->_aRef->GetHighLevelState() == Infected)
								InfectedAgents.AddAgent(curr->_aRef);

							curr = curr->_next;
						}

						// Determine if current cell has no infected to infect other cells
						if (InfectedAgents._nodeCounter == 0) {
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
								curr = _cellContainer.GetCellHead(r, c + 1);
							}
							break;
						case 1:
							// left
							if (c - 1 >= 0 ? _cellContainer.GetCounterValue(r, c - 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r, c - 1);
							}
							break;
						case 2:
							// top
							if (r + 1 < row_max ? _cellContainer.GetCounterValue(r + 1, c) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r + 1, c);
							}
							break;
						case 3:
							// top-right
							if (c + 1 < coln_max && r + 1 < row_max ? _cellContainer.GetCounterValue(r + 1, c + 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r + 1, c + 1);
							}
							break;
						case 4:
							// top-left
							if (c - 1 >= 0 && r + 1 < row_max ? _cellContainer.GetCounterValue(r + 1, c - 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r + 1, c - 1);
							}
							break;
						case 5:
							// bottom
							if (r - 1 >= 0 ? _cellContainer.GetCounterValue(r - 1, c) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r - 1, c);
							}
							break;
						case 6:
							// bottom-right
							if (c + 1 < coln_max && r - 1 >= 0 ? _cellContainer.GetCounterValue(r - 1, c + 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r - 1, c + 1);
							}
							break;
						case 7:
							// bottom-left
							if (c - 1 >= 0 && r - 1 >= 0 ? _cellContainer.GetCounterValue(r - 1, c - 1) > 0 : false) {
								//loop = true;
								curr = _cellContainer.GetCellHead(r - 1, c - 1);
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