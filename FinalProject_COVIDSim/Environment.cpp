#include "Environment.h"
#include <stdlib.h>
//---------------ENV--------------------------
bool Environment::STATScheduled = false;

// Agents arriving from another enviornment
void Environment::Arrive(Agent* a)
{
	_agentList.AddAgent(a, this);
}

// Random Movements
void Environment::MoveAgents()
{
	_cellContainer.~CellLinkedList();
	Location coordinate;
	
	// Get first agent Node 
	AgentContainer::Node* curr = _agentList.GetAgentHead();
	while(curr != nullptr) {
		// Calculating random movement
		coordinate._x = (rand() % _domain._x + curr->_a->GetLocation()._x) % _domain._x * _moveFrequency;
		coordinate._y = (rand() % _domain._y + curr->_a->GetLocation()._y) % _domain._y * _moveFrequency;
		curr->_a->SetLocation(coordinate);
		_cellContainer.AddAgent(curr->_a);

		curr = curr->_next;
	}
}

Environment * Environment::NextEnvironment()
{
	// If num environments next is zero, then return to self
	if (_numEnvironments > 0) {
		unsigned int RNG = rand() % 101;
		unsigned int i = -1;
		float temp = 0;
		do { 
			i++;
			temp = _nextEnvironmentProbabilities[i] + temp;
		} while (temp < RNG);
		return _nextEnvironments[i];
	}
	else
		return this;
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
void Environment::AgentContainer::AddAgent(Agent* a, Environment * env)
{
	// Creating new node
	Node* node = new Node(a);
	_numAgents++;
	
	// Point new node to head->next
	node->_next = _head->_next;

	// Head points at new node
	_head->_next = node;
	
	// Scheduling when Agent Leaves
	if(!_agentInEnvDuration)
		SimulationExecutive::GetInstance()->ScheduleEventIn(_agentInEnvDuration->GetRV(), new DepartEvent(_head, env));
}

Agent* Environment::AgentContainer::GetAndRemoveAgent(Node* previousNode)
{
	Agent* a = previousNode->_next->_a;
	Node* curr = previousNode->_next;
	previousNode->_next = curr->_next;

	// Deleting current node and reducing number of agents
	delete curr;
	curr = nullptr;
	_numAgents--;

	// returning the agent
	return a;
}

void Environment::AgentContainer::CreateAgents(unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent* SuscepibleEvent, InfectedStateEvent* InfectedEvent, Environment * env)
{
	Agent* a;
	Location coordinate;

	// Susceptible
	coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
	a = new Agent(coordinate, SuscepibleEvent, rand() % 100 + 1);
	if (env->_cellResolution > 0)
		env->_cellContainer.AddAgent(a);
	SuscepibleEvent->SetAgent(a);
	AddAgent(a, env);

	AgentEventAction* aea;
	for (int i = 1; i < numSusceptible; i++){
		coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
		aea = SuscepibleEvent->New();
		a = new Agent(coordinate, aea, rand() % 100 + 1);
		if (env->_cellResolution > 0)
			env->_cellContainer.AddAgent(a);
		aea->SetAgent(a);
		AddAgent(a, env);
	}

	// Infected
	coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
	a = new Agent(coordinate, InfectedEvent, rand() % 100 + 1);
	if (env->_cellResolution > 0)
		env->_cellContainer.AddAgent(a);
	InfectedEvent->SetAgent(a);
	AddAgent(a, env);

	for (int i = 1; i < numSusceptible; i++) {
		coordinate._x = rand() % env->_domain._x; coordinate._y = rand() % env->_domain._y; // randoming choosing location with in domain
		aea = InfectedEvent->New();
		a = new Agent(coordinate, InfectedEvent, rand() % 100 + 1);
		if (env->_cellResolution > 0)
			env->_cellContainer.AddAgent(a);
		aea->SetAgent(a);
		AddAgent(a, env);
	}
}

Environment::AgentContainer::~AgentContainer()
{
	// Node * current = m_head;
	while (_head != nullptr) {
		Node* to_delete = _head;
		_head = _head->_next;
		delete to_delete;
		to_delete = nullptr;
	}
	_numAgents = 0;
}

// Cell Data Structure
void Environment::CellLinkedList::CreateCells()
{
	// Initializing row and coln
	_numRows = 1 + ((_env->_domain._y - 1) / _env->_cellResolution);
	_numColns = 1 + ((_env->_domain._x - 1) / _env->_cellResolution);

	// initializing all the cells
	_cellHeads = new CellNode * *[_numRows];
	_cellCounter = new unsigned int* [_numRows];
	for (int i = 0; i < _numRows; i++) {
		_cellHeads[i] = new CellNode * [_numColns];
		_cellCounter[i] = new unsigned int[_numColns];
		for (int j = 0; j < _numColns; j++) {
			_cellHeads[i][j] = nullptr;
			_cellCounter[i][j] = 0;
		}
	}
}

void Environment::CellLinkedList::AddAgent(Agent* a)
{
	// Creating new node
	CellNode* newAgent = new CellNode(a);

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
	}
}
