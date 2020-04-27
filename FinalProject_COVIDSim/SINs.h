#pragma once
#include "AgentState.h"

// APPLICATION
// SusceptibleStateEvents
class SusceptibleStateEvent : public StateEvent {
public:
	SusceptibleStateEvent() {
		_a = nullptr;
		_highLevelState = Susceptible;
		_lowLevelState = "Susceptible";
		_stateStat = &STAT::GetInstance()->_numSusceptible;
	}

	SusceptibleStateEvent(Agent* a) {
		_a = a;
		_highLevelState = Susceptible;
		_lowLevelState = "Susceptible";
		_stateStat = &STAT::GetInstance()->_numSusceptible;
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW SusceptibleStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW SusceptibleStateEvent; }

	virtual void StateSpecificProcess();

	virtual bool StateInteractionProcess(Parameter* list);

	static bool SusceptibleRegister;
};

// InfectedStateEvents
class InfectedStateEvent : public StateEvent {
public:
	InfectedStateEvent() {
		_a = nullptr;
		_highLevelState = Infected;
		_lowLevelState = "Infected";
		_stateStat = &STAT::GetInstance()->_numInfected;
	}

	InfectedStateEvent(Agent* a) {
		_a = a;
		_highLevelState = Infected;
		_lowLevelState = "Infected";
		_stateStat = &STAT::GetInstance()->_numInfected;
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW InfectedStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW InfectedStateEvent; }

	virtual void StateSpecificProcess();

	static bool InfectedRegister;
};

// APPLICATION
class ExposedStateEvent : public InfectedStateEvent {
public:
	ExposedStateEvent() {
		_lowLevelState = "Exposed";
	}

	ExposedStateEvent(Agent* a) : InfectedStateEvent{ a } {
		_lowLevelState = "Exposed";
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW ExposedStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW ExposedStateEvent; }

private:
	static bool ExposedRegister;
};

class SymptomStateEvent : public InfectedStateEvent {
public:
	SymptomStateEvent() {
		_lowLevelState = "Symptom";
	}

	SymptomStateEvent(Agent* a) : InfectedStateEvent{ a } {
		_lowLevelState = "Symptom";
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW SymptomStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW SymptomStateEvent; }

	// Application
	virtual bool StateInteractionProcess(Parameter* list);

private:
	static bool SymptomRegister;
};

// NonSusceptibleStateEvents
class NonSusceptibleStateEvent : public StateEvent {
public:
	NonSusceptibleStateEvent() {
		_a = nullptr;
		_highLevelState = NonSusceptible;
		_lowLevelState = "NonSusceptible";
		_stateStat = &STAT::GetInstance()->_numNonSusceptible;
	}

	NonSusceptibleStateEvent(Agent* a) {
		_a = a;
		_highLevelState = NonSusceptible;
		_lowLevelState = "NonSusceptible";
		_stateStat = &STAT::GetInstance()->_numNonSusceptible;
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW NonSusceptibleStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW NonSusceptibleStateEvent; }

	virtual void StateSpecificProcess();

private:
	static bool NonSusceptibleRegister;
};

// APPLICATION
class RecoveredStateEvent : public NonSusceptibleStateEvent {
public:
	RecoveredStateEvent() {
		_lowLevelState = "Recovered";
	}

	RecoveredStateEvent(Agent* a) : NonSusceptibleStateEvent{ a } {
		_lowLevelState = "Recovered";
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW RecoveredStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW RecoveredStateEvent; }

	// Application
	virtual bool StateInteractionProcess(Parameter* list);

	static bool RecoveredRegister;
};

class DeadStateEvent : public NonSusceptibleStateEvent {
public:
	DeadStateEvent() {
		_lowLevelState = "Dead";
	}

	DeadStateEvent(Agent* a) : NonSusceptibleStateEvent{ a } {
		_lowLevelState = "Dead";
	}

	virtual AgentStateEventAction* New(Agent* a) { return DBG_NEW DeadStateEvent(a); }
	virtual AgentStateEventAction* New() { return DBG_NEW DeadStateEvent; }

	// Application
	virtual bool StateInteractionProcess(Parameter* list);

	static bool DeadRegister;
};



