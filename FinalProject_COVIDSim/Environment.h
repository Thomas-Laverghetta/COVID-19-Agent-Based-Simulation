#ifndef ENV_H
#define ENV_H
#include "AgentState.h"
#include <time.h>       /* time */

class Environment {
public:
	Environment(std::string EnvName, Distribution * agentInEnvDuration, float moveFrequency, unsigned int cellResolution, unsigned int Ymax, unsigned int Xmax, 
		unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent * initialHealthyState, InfectedStateEvent * initialInfectedState)
		: _envName(EnvName), _moveFrequency(moveFrequency), _cellResolution(cellResolution)
	{
		STAT::GetInstance()->ResetSTAT();
		_id = _nextId++;

		if (STAT::GetInstance()->_sample > 0) {
			_SIRoutputCompress.open("OutputData//SINs_outputCompress.txt", std::ios_base::app); _SIRoutputCompress << std::endl;
			//_SEIRoutput.open("OutputData//" + EnvName + "_SEIRoutput.txt", std::ios_base::app); _SEIRoutput << endl;
			_SEIRoutputCompressed.open("OutputData//SESRD_MultiEnv_outputCompressed.txt", std::ios_base::app);
			_statisticsFile.open("OutputData//Infection_Statistics.txt", std::ios_base::app);
		}
		else {
			_SIRoutputCompress.open("OutputData//SINs_outputCompress.txt");
			//_SEIRoutput.open("OutputData//" + EnvName + "_SEIRoutput.txt");
			_SEIRoutputCompressed.open("OutputData//SESRD_MultiEnv_outputCompressed.txt");
			_statisticsFile.open("OutputData//Infection_Statistics.txt");
			STAT::GetInstance()->_sample++;
		}

		// Setting domain
		_domain._x = Xmax;
		_domain._y = Ymax;

		if (_cellResolution > 0) {
			// Initializing and Creating cells
			_cellContainer.CreateCells();
		}
		_agentList.SettingDepartDistribution(agentInEnvDuration);

		// Next environment Parameters to null
		_nextEnvironments = nullptr;
		_nextEnvironmentProbabilities = nullptr;

		// Creating all Agents Simulation (WILL HAVE TO CHANGE LATER)
		_agentList.CreateAgents(numSusceptible, numInfected, initialHealthyState, initialInfectedState, this);

		// if move frequency is negative, then no movement (e.g., agent is at home and not interacting)
		if (_moveFrequency > 0)
			ScheduleEventAt(_moveFrequency, DBG_NEW UpdateEnvironmentEvent(this));
	}

	unsigned int GetNumAgentsInEnvironment();

	// Setting Next 
	void SetNextEnvironment(Environment** nextEnvArray, float* nextEnvProbs);
protected:
	//--------------------------------FUNCT---------------------------------
	void Arrive(Agent* a);
	virtual bool EnvironmentProcess() = 0;
	void MoveAgents();
	Environment * NextEnvironment();
	void PrintContentsOfEnvironment(Agent * a);
	
	//--------------------------------DATA_Struct----------------------------
	// List of agents for current environment
	class AgentContainer {
	public:
		AgentContainer() {
			_head = nullptr;
			_tail = nullptr;
			_numAgents = 0;
			_agentInEnvDuration = nullptr;
		}
		struct Node {
			Node(Agent* a) : _a(a), _next(nullptr){	}
			Agent* _a;
			Node* _next;

			~Node() {
				delete _a;
			}
		};

		// Setting Depart Time
		void SettingDepartDistribution(Distribution* agentInEnvDuration) {
			_agentInEnvDuration = agentInEnvDuration;
		}

		// Places Agent in list and Schedule Departure
		void AddAgent(Agent* a, Environment* env);

		// Will get nextNode after previousNode and remove from list
		Agent * RemoveAgent();
		
		// Creates Agents and schedules departure time 
		void CreateAgents(unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent* SuscepibleEvent, InfectedStateEvent* InfectedEvent, Environment * env);

		// Return the number of agents in environment
		unsigned int GetNumAgent() { return _numAgents; }

		// Return Head of list
		Node* GetAgentHead() {
			return _head;
		}

		void PrintAgents(Environment* env) {
			Node* curr = _head;
			// States
			if (curr) {
				do {
					env->PrintContentsOfEnvironment(curr->_a);
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
		Distribution* _agentInEnvDuration;
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
	AgentContainer _agentList;
	Location _domain;
	unsigned int _cellResolution;
	Time _moveFrequency;
	CellLinkedList _cellContainer{ this };

	// Next Environment
	Environment** _nextEnvironments;
	float* _nextEnvironmentProbabilities;

	unsigned int _id;
	static unsigned int _nextId;
	std::ofstream _SIRoutputCompress;
	std::ofstream _SEIRoutputCompressed;
	std::ofstream _statisticsFile;

	virtual ~Environment() {
		delete _nextEnvironmentProbabilities;
	}
private:
	//--------------------------------Events---------------------------------
	class UpdateEnvironmentEvent : public EventAction {
	public:
		UpdateEnvironmentEvent(Environment* env) : _env(env)
		{}

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