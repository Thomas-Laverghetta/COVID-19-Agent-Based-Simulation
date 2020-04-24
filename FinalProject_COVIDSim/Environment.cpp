#include "Environment.h"
#include <stdlib.h>
#include <iomanip>
unsigned int Environment::_nextId = 0;
//---------------ENV--------------------------
bool Environment::_StatEventSch = false;

// Agents arriving from another enviornment
void Environment::Arrive(Agent* a)
{
	// Setting Random location in environment
	Location coordinate;
	coordinate._x = rand() % _domain._x; coordinate._y = rand() % _domain._y;
	a->SetLocation(coordinate);

	_agentList.AddAgent(a, this);
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
			if (rand() % 4 == 1) {
				// Calculating random movement
				coordinate._x = ((rand() % 5 + curr->_a->GetLocation()._x) * (int) _moveFrequency) % _domain._x;
				coordinate._y = ((rand() % 5 + curr->_a->GetLocation()._y) * (int)_moveFrequency) % _domain._y ;
			}
			else if (rand() % 4 == 2) {
				// Calculating random movement
				coordinate._x = (abs((int)(rand() % 5 - curr->_a->GetLocation()._x)) * (int)_moveFrequency) % _domain._x;
				coordinate._y = (abs((int)(rand() % 5 - curr->_a->GetLocation()._y)) * (int)_moveFrequency)  % _domain._y;
			}
			else if (rand() % 4 == 3) {
				// Calculating random movement
				coordinate._x = (abs((int)(rand() % 5 - curr->_a->GetLocation()._x)) * (int)_moveFrequency) % _domain._x;
				coordinate._y = ((int)(rand() % 5 + curr->_a->GetLocation()._y) * (int)_moveFrequency) % _domain._y;
			}
			else {
				// Calculating random movement
				coordinate._x = ((int)(rand() % 5 + curr->_a->GetLocation()._x) * (int)_moveFrequency) % _domain._x;
				coordinate._y = (abs((int)(rand() % 5 - curr->_a->GetLocation()._y)) * (int)_moveFrequency) % _domain._y;
			}

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
	_SIRoutputCompress << a->GetId() << "," << a->GetLocation()._x << "," << a->GetLocation()._y << "," << _id << "," << a->GetHighLevelState() << std::endl;

	_SEIRoutputCompressed << a->GetId() << "," << a->GetLocation()._x << "," << a->GetLocation()._y << "," << _id << "," << a->GetHighLevelState() << "," << a->GetLowLevelState() << std::endl;

	//_SEIRoutput << "| Agent" << setw(5) << a->GetId() << " | Age " << a->GetAge() << " | Location (x,y) = (" << setw(4) <<
	//	a->GetLocation()._x << "," << setw(4) << a->GetLocation()._y << ") in Environment - " << setw(3) << _id << " | Low-Level-State " <<
	//	a->GetLowLevelState() << endl;
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
void Environment::AgentContainer::AddAgent(Agent* a, Environment* env)
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

	// Scheduling when Agent Leaves
	// If _agentInEnvDuration is null then this is a terminating environment
	if(_agentInEnvDuration)
		ScheduleEventIn(_agentInEnvDuration->GetRV(), DBG_NEW DepartEvent(env));
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
	_numAgents--;
	return a;
}

void Environment::AgentContainer::CreateAgents(unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent* SuscepibleEvent, InfectedStateEvent* InfectedEvent, Environment * env)
{
	Agent* a;
	Location coordinate;
	AgeDistribution ageDistr;

	if (numSusceptible > 0) {
		// Susceptible
		coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
		a = DBG_NEW Agent(coordinate, SuscepibleEvent->New(), ageDistr.GetRV());
		if (env->_cellResolution > 0)
			env->_cellContainer.AddAgent(a);
		AddAgent(a, env);

		for (int i = 1; i < numSusceptible; i++) {
			coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
			a = DBG_NEW Agent(coordinate, SuscepibleEvent->New(), ageDistr.GetRV());
			if (env->_cellResolution > 0)
				env->_cellContainer.AddAgent(a);
			AddAgent(a, env);
		}
	}

	// Infected
	if (numInfected > 0) {
		coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
		a = DBG_NEW Agent(coordinate, InfectedEvent->New(), ageDistr.GetRV());
		if (env->_cellResolution > 0)
			env->_cellContainer.AddAgent(a);
		AddAgent(a, env);

		for (int i = 1; i < numInfected; i++) {
			coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
			a = DBG_NEW Agent(coordinate, InfectedEvent->New(), ageDistr.GetRV());
			if (env->_cellResolution > 0)
				env->_cellContainer.AddAgent(a);
			AddAgent(a, env);
		}
	}

	//PrintAgents(env);
	//STAT::GetInstance()->printSTAT(env->_SIRoutputCompress);
	//STAT::GetInstance()->printSpecificSTAT(env->_SEIRoutput);
	//STAT::GetInstance()->printSpecificSTATCompressed(env->_SEIRoutputCompressed);
	//env->MoveAgents();
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
