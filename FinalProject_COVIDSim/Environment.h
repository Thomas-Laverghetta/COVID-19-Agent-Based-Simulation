#ifndef ENV_H
#define ENV_H
#include "AgentState.h"
#include <time.h>       /* time */

class Environment {
public:
	Environment(std::string EnvName, Distribution * agentInEnvDuration, float moveFrequency) : _envName(EnvName), _moveFrequency(moveFrequency) 
	{
		if (!STATScheduled) {
			/* initialize random seed: */
			srand(time(NULL));
			ScheduleEventAt(1.0f, new STATEvent);
			STATScheduled = true;
		}

		_agentList.SettingDepartDistribution(agentInEnvDuration);

		// if move frequency is negative, then no movement (e.g., agent is at home and not interacting)
		if (_moveFrequency >= 0)
			ScheduleEventAt(0, new MoveEvent(this));
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
	
	
	//--------------------------------DATA_Struct----------------------------
	// List of agents for current environment
	class AgentContainer {
	public:
		AgentContainer() {
			// Zero Node
			_head = new Node;
		}
		struct Node {
			Node() : _a(nullptr), _next(nullptr) { }
			Node(Agent* a) : _a(a), _next(nullptr){	}
			Agent* _a;
			Node* _next;
		};

		// Setting Depart Time
		void SettingDepartDistribution(Distribution* agentInEnvDuration) {
			_agentInEnvDuration = agentInEnvDuration;
		}

		// Places Agent in list and Schedule Departure
		void AddAgent(Agent* a, Environment * env);

		// Will get nextNode after previousNode and remove from list
		Agent * GetAndRemoveAgent(Node* previousNode);
		
		// Creates Agents and schedules departure time 
		void CreateAgents(unsigned int numSusceptible, unsigned int numInfected, SusceptibleStateEvent* SuscepibleEvent, InfectedStateEvent* InfectedEvent, Environment * env);

		// Return the number of agents in environment
		unsigned int GetNumAgent() { return _numAgents; }

		// Return Head of list
		Node* GetAgentHead() {
			return _head->_next;
		}

		// destructor
		~AgentContainer();
	private:
		Node* _head;
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

	//--------------------------------Events---------------------------------
	class MoveEvent : public EventAction {
	public:
		MoveEvent(Environment* env) : _env(env)
		{}
		
		void Execute() {
			// Moving Agnets
			_env->MoveAgents();

			// Running any specific Environment process
			_env->EnvironmentProcess();

			// Update any influences
			_env->_envDI->UpdateInfluences(); 

			// Scheduling next move
			ScheduleEventIn(_env->_moveFrequency, this);
		}
	private:
		Environment* _env;
	};
	class DepartEvent : public EventAction {
	public:
		DepartEvent(AgentContainer::Node* previousNode, Environment* env) 
			: _previousNode(previousNode), _env(env) {}

		// Gets agent from list, then sends agent to next environment
		void Execute() {
			_env->NextEnvironment()->Arrive(_env->_agentList.GetAndRemoveAgent(_previousNode));
		}
	private:
		AgentContainer::Node* _previousNode;
		Environment* _env;
	};

	// Variables
	std::string _envName;
	AgentContainer _agentList;
	Location _domain;
	unsigned int _cellResolution;
	Time _moveFrequency;
	CellLinkedList _cellContainer{ this };
	DiseaseInfluence* _envDI;

	// Next Environment
	Environment** _nextEnvironments;
	float* _nextEnvironmentProbabilities;

private:
	// Call STAT Update every cycle
	class STATEvent : public EventAction {
	public:
		STATEvent() {}
		void Execute() {
			STAT::printSTAT();
			ScheduleEventIn(1.0f, this);
		}
	};

	// Determing if STAT Event loop was executed
	static bool STATScheduled;
};
#endif // !ENV_H