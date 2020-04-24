#ifndef ENV_H
#define ENV_H
#include "AgentState.h"
#include <time.h>       /* time */

class Environment {
public:
	Environment(std::string EnvName, Variant * agentInEnvDuration, float moveFrequency, unsigned int cellResolution, unsigned int Ymax, unsigned int Xmax, 
		unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent * initialHealthyState, InfectedStateEvent * initialInfectedState)
		: _envName(EnvName), _moveFrequency(moveFrequency), _cellResolution(cellResolution)
	{
		STAT::GetInstance()->ResetSTAT();
		_id = _nextId++;

		// Setting domain
		_domain._x = Xmax;
		_domain._y = Ymax;

		if (_cellResolution > 0) {
			// Initializing and Creating cells
			_cellContainer.CreateCells();
		}
		_agentList.SettingDepartVariant(agentInEnvDuration);

		// Next environment Parameters to null
		_nextEnvironments = nullptr;
		_nextEnvironmentProbabilities = nullptr;

		// if move frequency is negative, then no movement (e.g., agent is at home and not interacting)
		if (_moveFrequency > 0)
			ScheduleEventAt(_moveFrequency, DBG_NEW UpdateEnvironmentEvent(this));

		// Creating all Agents Simulation (WILL HAVE TO CHANGE LATER)
		_agentList.CreateAgents(numSusceptible, numInfected, initialHealthyState, initialInfectedState);
	}

	unsigned int GetNumAgentsInEnvironment();

	// Setting Next 
	void SetNextEnvironment(Environment** nextEnvArray, float* nextEnvProbs);
protected:
	//--------------------------------FUNCT---------------------------------
	void Arrive(Agent* a);
	virtual void EnvironmentProcess() = 0;
	void MoveAgents();
	Environment * NextEnvironment();
	void PrintContentsOfEnvironment(Agent * a);
	void CheckAgentDistances();
	
	//--------------------------------DATA_Struct----------------------------
	// List of agents for current environment
	class AgentContainer {
	public:
		AgentContainer(Environment * env) : _env (env) {
			_head = nullptr;
			_tail = nullptr;
			_numAgents = 0;
			_agentInEnvDuration = nullptr;
			_addRemoveStat = false;
		}

		void SetAddRemoveStat(bool stat) {
			_addRemoveStat = stat;
		}

		struct Node {
			Node(Agent* a) : _a(a), _next(nullptr){	}
			Agent* _a;
			Node* _next;
		};

		// Setting Depart Time
		void SettingDepartVariant(Variant* agentInEnvDuration) {
			_agentInEnvDuration = agentInEnvDuration;
		}

		// Places Agent in list and Schedule Departure
		void AddAgent(Agent* a);

		// Will get nextNode after previousNode and remove from list
		Agent * RemoveAgent();
		
		// Creates Agents and schedules departure time 
		void CreateAgents(unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent* SuscepibleEvent, InfectedStateEvent* InfectedEvent);

		// Return the number of agents in environment
		unsigned int GetNumAgent() { return _numAgents; }

		// Return Head of list
		Node* GetAgentHead() {
			return _head;
		}

		void PrintAgents() {
			Node* curr = _head;
			// States
			if (curr) {
				do {
					_env->PrintContentsOfEnvironment(curr->_a);
					curr = curr->_next;

				} while (curr != _head);
			}
		}

		// destructor
		~AgentContainer();
	private:
		Node* _head; 
		Node* _tail;
		unsigned int _numAgents;
		Variant* _agentInEnvDuration;
		bool _addRemoveStat;
		Environment* _env;
	};
	
	// Cell data Structure
	class CellLinkedList {
	public:
		// Agent 
		CellLinkedList(Environment* env) {
			_env = env;
		}

		// Node
		class CellNode {
		public:
			CellNode* _next;
			Agent* _aRef;
			CellNode(Agent* a) { _aRef = a; _next = nullptr; }
		};

		// creates all cells for simulation
		void CreateCells();

		// Adding Agent
		void AddAgent(Agent* a);

		// Deleting all Nodes
		~CellLinkedList();
		
		// Resetting Environment Cells
		void ResetLinkedList();

		// Getting head node from linked list
		CellNode* GetCellHead(unsigned int row, unsigned int coln) {
			return _cellHeads[row][coln];
		}

		unsigned int GetCounterValue(unsigned int row, unsigned int coln) {
			return _cellCounter[row][coln];
		}
	private:
		Environment* _env;
		CellNode*** _cellHeads;
		unsigned int** _cellCounter;
		unsigned int _numRows;
		unsigned int _numColns;
	};

	// Variables
	std::string _envName;
	AgentContainer _agentList{ this };
	Location _domain;
	unsigned int _cellResolution;
	Time _moveFrequency;
	CellLinkedList _cellContainer{ this };

	// Next Environment
	Environment** _nextEnvironments;
	float* _nextEnvironmentProbabilities;

	unsigned int _id;
	static unsigned int _nextId;
	static std::ofstream _SIRoutputCompress;
	static std::ofstream _SEIRoutputCompressed;
	static std::ofstream _statisticsFile;

	virtual ~Environment() {
		delete _nextEnvironmentProbabilities;
	}
private:
	//--------------------------------Events---------------------------------
	class UpdateEnvironmentEvent : public EventAction {
	public:
		UpdateEnvironmentEvent(Environment* env) : _env(env)
		{}

		// Control loops
		void Execute() {
			// Running any specific Environment process
			_env->EnvironmentProcess();

			// Scheduling next move
			if (STAT::GetInstance()->_numInfected > 0)
				ScheduleEventIn(_env->_moveFrequency, this);
			else if(!_StatEventSch) {
				ScheduleEventIn(0, DBG_NEW StatEvent(_env));
				_StatEventSch = true;
			}

			// Diplay Statistics
			STAT::GetInstance()->printSIRTallySTAT(_env->_SIRoutputCompress);
			STAT::GetInstance()->printSEIRTallySTAT(_env->_SEIRoutputCompressed);
		}
	private:
		Environment* _env;
	};
	class DepartEvent : public EventAction {
	public:
		DepartEvent(Environment* env)
			: _env(env) {}

		// Gets agent from list, then sends agent to next environment
		void Execute() {
			_env->NextEnvironment()->Arrive(_env->_agentList.RemoveAgent());
		}
	private:
		Environment* _env;
	};

	class StatEvent : public EventAction {
	public:
		StatEvent(Environment* env) : _env(env) {}

		void Execute() {
			STAT::GetInstance()->CumStatistics(_env->_statisticsFile);
			_env->_StatEventSch = false;
			_env = nullptr;
		}
	private:
		Environment* _env;
	};

	static bool _StatEventSch;
};
#endif // !ENV_H