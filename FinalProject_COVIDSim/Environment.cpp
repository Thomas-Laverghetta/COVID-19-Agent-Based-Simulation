#include "Environment.h"
#include <stdlib.h>
#include <iomanip>
unsigned int Environment::_nextId = 0;
//---------------ENV--------------------------
bool Environment::_StatEventSch = false;
std::ofstream Environment::_SIRoutputCompress = std::ofstream("OutputData//SINs_outputCompress.txt");
std::ofstream Environment::_SEIRoutputCompressed = std::ofstream("OutputData//SESRD_outputCompressed.txt");
std::ofstream Environment::_statisticsFile = std::ofstream("OutputData//Infection_Statistics.txt");

// Agents arriving from another enviornment
void Environment::Arrive(Agent* a)
{
	// Setting Random location in environment
	Location coordinate;
	coordinate._x = rand() % _domain._x; coordinate._y = rand() % _domain._y;
	a->SetLocation(coordinate);

	_agentList.AddAgent(a);
}

// Random Movements
void Environment::MoveAgents()
{
	_cellContainer.ResetLinkedList();
	Location coordinate;
	
	// States
	//Normal moveDist(0, 6.0f);

	// Get first agent Node 
	AgentContainer::Node* curr = _agentList.GetAgentHead();
	if (curr) {
		do {
			coordinate = curr->_a->GetLocation();
			if (rand() % 2 == 1) {
				// Calculating random movement
				coordinate._x += rand() % 5 * (int) _moveFrequency;
			}
			else
				coordinate._x -= rand() % 5 * (int)_moveFrequency;

			if (rand() % 2 == 1)
				coordinate._y += rand() % 5 * (int)_moveFrequency;
			else
				coordinate._y -= rand() % 5 * (int)_moveFrequency;

			//else if (rand() % 4 == 2) {
			//	// Calculating random movement
			//}
			//else if (rand() % 4 == 3) {
			//	// Calculating random movement
			//	coordinate._x -= rand() % 5 * (int)_moveFrequency;
			//	coordinate._y += rand() % 5 * (int)_moveFrequency;
			//}
			//else {
			//	// Calculating random movement
			//	coordinate._x += rand() % 5 * (int)_moveFrequency;
			//	coordinate._y -= rand() % 5 * (int)_moveFrequency;
			//}

			// Normalizing between Xmax and Ymax
			int abs_x = abs((int)(_domain._x - (int)coordinate._x));
			int abs_y = abs((int)(_domain._y - (int)coordinate._y));

			if (abs_x < _domain._x)
				coordinate._x = _domain._x - abs_x;
			else
				coordinate._x = abs_x - _domain._x;
			
			if (abs_y < _domain._y)
				coordinate._y = _domain._y - abs_y;
			else
				coordinate._y = abs_y - _domain._y;

			//coordinate._x = abs((int)(moveDist.GetRV() + curr->_a->GetLocation()._x)) % _domain._x * _moveFrequency;
			//coordinate._y = abs((int)(moveDist.GetRV() + curr->_a->GetLocation()._y)) % _domain._y * _moveFrequency;

			/*coordinate._x = (rand() % _domain._x + curr->_a->GetLocation()._x) % _domain._x * _moveFrequency;
			coordinate._y = (rand() % _domain._y + curr->_a->GetLocation()._y) % _domain._y * _moveFrequency;*/

			curr->_a->SetLocation(coordinate);
			_cellContainer.AddAgent(curr->_a);

			PrintContentsOfEnvironment(curr->_a);

			curr = curr->_next;
		} while (curr != _agentList.GetAgentHead());
	}
}

Environment * Environment::NextEnvironment()
{
	// If num environments next is zero, then return to self
	if (_nextEnvironments) {
		unsigned int RNG = rand() % 101;
		unsigned int i = -1;
		float temp = 0;
		do { 
			i++;
			temp = _nextEnvironmentProbabilities[i] + temp;
		} while (temp*100 < RNG);
		return _nextEnvironments[i];
	}
	else
		return this;
}

void Environment::PrintContentsOfEnvironment(Agent* a)
{
	// Num
	std::string lowLvl = a->GetLowLevelState();
	int lowLvlInt;
	if (lowLvl == "Susceptible")
		lowLvlInt = 0;
	else if (lowLvl == "Exposed")
		lowLvlInt = 1;
	else if (lowLvl == "Symptom")
		lowLvlInt = 2;
	else if (lowLvl == "Recovered")
		lowLvlInt = 3;
	else
		lowLvlInt = 4;


	_SIRoutputCompress << a->GetId() << "," << a->GetLocation()._x << "," << a->GetLocation()._y << "," << _id << "," << a->GetHighLevelState() << std::endl;

	_SEIRoutputCompressed << a->GetId() << "," << a->GetLocation()._x << "," << a->GetLocation()._y << "," << _id << "," << a->GetHighLevelState() << "," << lowLvlInt << std::endl;

	//_SEIRoutput << "| Agent" << setw(5) << a->GetId() << " | Age " << a->GetAge() << " | Location (x,y) = (" << setw(4) <<
	//	a->GetLocation()._x << "," << setw(4) << a->GetLocation()._y << ") in Environment - " << setw(3) << _id << " | Low-Level-State " <<
	//	a->GetLowLevelState() << endl;
}

// For Check Distance
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
		NodeTracker* newNode = DBG_NEW NodeTracker(a);
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
	if (STAT::GetInstance()->_numSusceptible > 0 && STAT::GetInstance()->_numInfected > 0) {
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
						Distance* distance = DBG_NEW Distance(InfectedAgents._nodeCounter);
						AgentTracker::NodeTracker* curr_H = HealthyAgents.GetHead();
						AgentTracker::NodeTracker* curr_I = InfectedAgents.GetHead();

						// Calculating the distance between healthy and infected
						while (curr_H != nullptr) {
							while (curr_I != nullptr) {
								// Calculating Distance
								distance->AddDistance(sqrtf((curr_H->_aRef->GetLocation()._x - curr_I->_aRef->GetLocation()._x) *
									(curr_H->_aRef->GetLocation()._x - curr_I->_aRef->GetLocation()._x) +
									(curr_H->_aRef->GetLocation()._y - curr_I->_aRef->GetLocation()._y) *
									(curr_H->_aRef->GetLocation()._y - curr_I->_aRef->GetLocation()._y)),
									curr_I->_aRef->GetLowLevelState());
								curr_I = curr_I->_next;
							}
							curr_H->_aRef->AgentInteraction(distance);
							curr_I = InfectedAgents.GetHead();
							curr_H = curr_H->_next;
							distance->resetIndex(); // resets index on float[] for next Healthy agent
						}
						delete distance;
					}
				}
			}
		}
	}
}

unsigned int Environment::GetNumAgentsInEnvironment()
{
	return _agentList.GetNumAgent();
}

void Environment::SetNextEnvironment(Environment** nextEnvArray, float* nextEnvProbs)
{
	_nextEnvironments = nextEnvArray;
	_nextEnvironmentProbabilities = nextEnvProbs;
}

//--------------------------------DATA_Struct----------------------------
// Agent Container
void Environment::AgentContainer::AddAgent(Agent* a)
{
	// Creating new node
	Node* node = DBG_NEW Node(a);
	_numAgents++;
	
	// If no agents in list
	if (!_head) {
		_tail = _head = node;
		_tail->_next = _head;
	}
	else {
		// Point new node to head->next
		node->_next = _head->_next;

		// Head points at new node
		_head->_next = node;

		// If head and tail are pointing the same node
		if (_head == _tail)
			_tail = _head->_next;
	}

	if (_addRemoveStat) {
		PrintAgents();
		// Diplay Statistics
		STAT::GetInstance()->printSIRTallySTAT(_env->_SIRoutputCompress);
		STAT::GetInstance()->printSEIRTallySTAT(_env->_SEIRoutputCompressed);
	}
	// Scheduling when Agent Leaves
	// If _agentInEnvDuration is null then this is a terminating environment
	if(_agentInEnvDuration)
		ScheduleEventIn(_agentInEnvDuration->GetRV(), DBG_NEW DepartEvent(_env));
}

Agent* Environment::AgentContainer::RemoveAgent()
{
	Agent* a = _head->_a;
	if (!(_tail == _head)) {
		_tail->_next = _head->_next;
		delete _head;
		_head = _tail->_next;
	}
	else {
		delete _head;
		_tail = nullptr;
		_head = nullptr;
	}

	if (_addRemoveStat) {
		PrintAgents();
		// Diplay Statistics
		STAT::GetInstance()->printSIRTallySTAT(_env->_SIRoutputCompress);
		STAT::GetInstance()->printSEIRTallySTAT(_env->_SEIRoutputCompressed);
	}

	_numAgents--;
	return a;
}

void Environment::AgentContainer::CreateAgents(unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent* SuscepibleEvent, InfectedStateEvent* InfectedEvent)
{
	Agent* a;
	Location coordinate;
	AgeVariant ageDistr;

	if (numSusceptible > 0) {
		// Susceptible
		coordinate._x = rand() % _env->_domain._x; coordinate._y = rand() % _env->_domain._y; // randoming choosing location with in domain
		a = DBG_NEW Agent(coordinate, SuscepibleEvent->New(), ageDistr.GetRV());
		if (_env->_cellResolution > 0)
			_env->_cellContainer.AddAgent(a);
		AddAgent(a);

		for (int i = 1; i < numSusceptible; i++) {
			coordinate._x = rand() % _env->_domain._x; coordinate._y = rand() % _env->_domain._y; // randoming choosing location with in domain
			a = DBG_NEW Agent(coordinate, SuscepibleEvent->New(), ageDistr.GetRV());
			if (_env->_cellResolution > 0)
				_env->_cellContainer.AddAgent(a);
			AddAgent(a);
		}
	}

	// Infected
	if (numInfected > 0) {
		coordinate._x = rand() % _env->_domain._x; coordinate._y = rand() % _env->_domain._y; // randoming choosing location with in domain
		a = DBG_NEW Agent(coordinate, InfectedEvent->New(), ageDistr.GetRV());
		if (_env->_cellResolution > 0)
			_env->_cellContainer.AddAgent(a);
		AddAgent(a);

		for (int i = 1; i < numInfected; i++) {
			coordinate._x = rand() % _env->_domain._x; coordinate._y = rand() % _env->_domain._y; // randoming choosing location with in domain
			a = DBG_NEW Agent(coordinate, InfectedEvent->New(), ageDistr.GetRV());
			if (_env->_cellResolution > 0)
				_env->_cellContainer.AddAgent(a);
			AddAgent(a);
		}
	}
}

Environment::AgentContainer::~AgentContainer()
{
	// Node * current = m_head;
	if (!_head)
		return;

	Node* traversePointer = _head;
	Node* deletePointer;
	do {
		deletePointer = traversePointer;
		traversePointer = traversePointer->_next;

		// deleting the agent instance
		delete deletePointer->_a;
		delete deletePointer;
	} while (traversePointer != _head);
	_head = nullptr;
	_numAgents = 0;
}

// Cell Data Structure
void Environment::CellLinkedList::CreateCells()
{
	// Initializing row and coln
	_numRows = 1 + ((_env->_domain._y - 1) / _env->_cellResolution);
	_numColns = 1 + ((_env->_domain._x - 1) / _env->_cellResolution);

	// initializing all the cells
	_cellHeads = DBG_NEW CellNode * *[_numRows];
	_cellCounter = DBG_NEW unsigned int* [_numRows];
	for (int i = 0; i < _numRows; i++) {
		_cellHeads[i] = DBG_NEW CellNode * [_numColns];
		_cellCounter[i] = DBG_NEW unsigned int[_numColns];
		for (int j = 0; j < _numColns; j++) {
			_cellHeads[i][j] = nullptr;
			_cellCounter[i][j] = 0;
		}
	}
}

void Environment::CellLinkedList::AddAgent(Agent* a)
{
	// Creating new node
	CellNode* newAgent = DBG_NEW CellNode(a);

	// Finding cell location and getting head
	unsigned int coln = a->GetLocation()._x / _env->_cellResolution;
	unsigned int row = a->GetLocation()._y / _env->_cellResolution;

	_cellCounter[row][coln]++;

	// Placing Agent in cell
	newAgent->_next = _cellHeads[row][coln];
	_cellHeads[row][coln] = newAgent;
}

Environment::CellLinkedList::~CellLinkedList()
{
	// Node * current = m_head;
	for (int row = 0; row < _numRows; row++) {
		for (int coln = 0; coln < _numColns; coln++) {
			while (_cellHeads[row][coln] != nullptr) {
				CellNode* to_delete = _cellHeads[row][coln];
				_cellHeads[row][coln] = _cellHeads[row][coln]->_next;
				delete to_delete;
				to_delete = nullptr;
				_cellCounter[row][coln] = 0;
			}
		}
		delete _cellCounter[row];
		delete _cellHeads[row];
	}
	delete[] _cellHeads; _cellHeads = nullptr;
	delete[] _cellCounter; _cellCounter = nullptr;
}

void Environment::CellLinkedList::ResetLinkedList()
{
	// Node * current = m_head;
	for (int row = 0; row < _numRows; row++) {
		for (int coln = 0; coln < _numColns; coln++) {
			while (_cellHeads[row][coln] != nullptr) {
				CellNode* to_delete = _cellHeads[row][coln];
				_cellHeads[row][coln] = _cellHeads[row][coln]->_next;
				delete to_delete;
				to_delete = nullptr;
				_cellCounter[row][coln] = 0;
			}
		}
	}
}
