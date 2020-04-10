#ifndef ENV_H
#define ENV_H
#include "SimObj.h"
#include "AgentState.h"
#include <time.h>       /* time */

//---------ENV-----------------
class Environment : public SimObj
{
public:
	/*
		Creates Agents and Sets ranges
	*/
	Environment(unsigned int numAgents = 100, unsigned int numInfected = 3, unsigned int Ymax = 100, unsigned int Xmax = 100, 
				unsigned int cellResolution = 10, Time moveFrequency = 1.0f, DiseaseInfluence* DI = nullptr);
private:
	//--------------------------------DATA_Struct----------------------------
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
		void CreateCells() {
			// Initializing row and coln
			_numRows = 1 + ((_env->_domain._y - 1) / _env->_cellResolution);
			_numColns = 1 + ((_env->_domain._x - 1) / _env->_cellResolution);

			// initializing all the cells
			_cellHeads = new CellNode** [_numRows];
			_cellCounter = new unsigned int* [_numRows];
			for (int i = 0; i < _numRows; i++) {
				_cellHeads[i] = new CellNode * [_numColns];
				_cellCounter[i] = new unsigned int [_numColns];
				for (int j = 0; j < _numColns; j++) {
					_cellHeads[i][j] = nullptr;
					_cellCounter[i][j] = 0;
				}
			}
		}

		// Adding Agent
		void AddAgent(Agent* a) {
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

		// Deleting all Nodes
		~CellLinkedList() {
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

	//------------------------------STATE_VAR------------------------------
	Agent** _agentRef;
	CellLinkedList _cellContainer{ this };
	Time _moveFrequency;
	DiseaseInfluence* _envDI;
	Location _domain;
	unsigned int _cellResolution;
	unsigned int _numAgents;

	//--------STAT--------------
	class STAT {
	public:
		static unsigned int _numInfected;
		static unsigned int _numSusceptible;
		static unsigned int _numOther;
		static Environment* _env;
		static void printSTAT() {
			printf("S = %i | I = %i | R = %i | ", _numSusceptible, _numInfected, _numOther);
		}
		// will print or update any data after each move
		static void UpdateData();
	};

	//--------------------------------Events---------------------------------
	// Movement Event
	class MoveEvent : public EventAction, SimObj {
	public:
		inline MoveEvent(Environment* env) : _env(env) {};

		void Execute() {
			_env->MoveAgents();
			_env->CheckAgentDistances();
			SimulationExecutive::GetInstance();
			STAT::UpdateData();
			if(STAT::_numInfected > 0)
				ScheduleEventIn(_env->_moveFrequency, this);
		}
	private:
		Environment* _env;
	};

	//--------------------------------FUNCT---------------------------------
	void CheckAgentDistances();
	void MoveAgents();
};
#endif // !ENV_H