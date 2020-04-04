#ifndef ENV_H
#define ENV_H
#include "SimObj.h"
#include "AgentState.h"
//---------ENV-----------------
class Environment : public SimObj
{
public:
	/*
		Creates Agents and Sets ranges
	*/
	Environment(unsigned int numAgents, unsigned int numInfected, unsigned int Ymax, unsigned int Xmax, unsigned int cellResolution, Time stepSize, DiseaseInfluence* DI);
private:
	//--------------------------------DATA_Struct----------------------------
	// Cell data Structure
	struct CellNode {
		CellNode* _next = nullptr;
		Agent* _aRef = nullptr;
	};

	//------------------------------STATE_VAR------------------------------
	Agent** _agentRef;
	CellNode*** _cell;
	unsigned int** _cellCounter;
	Time _stepSize;
	DiseaseInfluence* _envDI;
	unsigned int _Ymax, _Xmax, _cellResolution;

	//--------------------------------Events---------------------------------
	// Movement Event
	class MoveEvent : public EventAction, SimObj {
	public:
		inline MoveEvent(Environment* env) : _env(env) {};

		void Execute() {
			_env->MoveAgents();
			_env->AgentStatechange(); 
			ScheduleEventIn(_env->_stepSize, new MoveEvent(_env));
		}
	private:
		Environment* _env;
	};

	//--------------------------------FUNCT---------------------------------
	void MoveAgents();
	void AgentStatechange();

};
#endif // !ENV_H