//***** compute average distance travelled to insert

#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>
#include "SimulationExecutive.h"
#include "Stopwatch.h"
#define timing 1 //(0 = none,  1 = ev exe, 2 = EEL sch, 3 = SEL sch)
//#define eventStats

using namespace stopwatch;
using namespace std;

/*Current Goals
- Change from standard linked list to calendar queue
*/


ofstream StatData::_statData;

class EventStat
{
public:
	EventStat()
	{
		_maxExeQ = INT_MIN;
		_minExeQ = INT_MAX;
		_avgExeQ = 0.0;
		_exeQsum = 0.0;


		_maxSchQ = INT_MIN;
		_minSchQ = INT_MAX;
		_avgSchQ = 0.0;
		_schQsum = 0.0;

		_totalMinQ = INT_MAX;
		_totalMaxQ = INT_MIN;
		_totalAvgQ = 0.0;
		_numScheduled = 0.0;

		_exeEvents = 0;
		_numExeScheduled = 0.0;
		_numSchScheduled = 0.0;

		testExe = 0.0;
		testSch = 0.0;
	}

	void AverageEventsQ()
	{
		testExe = double(_exeQsum) / double(_numExeScheduled);
		testSch = double(_schQsum) / double(_numSchScheduled);
	}

	int _exeEvents; // total number of events executed
	int _maxExeQ; //Maximum events in Execute Queue after Steady State
	int _minExeQ; //Minimum events in Execute Queue after Steady State
	double _numExeScheduled;
	double _avgExeQ; //Average events in Execute Queue after Steady State
	double _exeQsum;
	int _maxSchQ; //Maximum events in Schedule Queue after Steady State
	int _minSchQ; //Minimum events in Schedule Queue after Steady State
	double _numSchScheduled;
	double _avgSchQ; //Average events in Schedule Queue after Steady State
	double _schQsum;

	double _totalMinQ; //Minimum total number of events in system
	double _totalMaxQ; //Maximum total number of events in system
	double _totalAvgQ; //Average number of event in system
	double _numScheduled; //Total Number of events scheduled

	double testExe;
	double testSch;
};

class SimulationExecutive
{
public:
	static Time GetSimulationTime() { return _simTime; }

	static void FlipSteady() { _steady = !_steady; };

	static void RunSimulation(Time endTime)
	{
		//Steady state determined by Time

		_schDec = false;
		_terminate = false;
		_eventStats = DBG_NEW EventStat(); // Statistics reset between runs

		//If not steady state
		if (!_steady)
		{
			_simTime = 0.0;

			//SWAP EVENTS FROM SEL TO EEL if first run
			_schEventList.SwapLists(_exeEventList);	
		}

		_scheduleThread = DBG_NEW thread(startScheduleThread);//Start Schedule Thread
		startExecuteThread(endTime); //Start Execute Thread

		_scheduleThread->join();//Join threads for termination

		if (_steady && (timing > 0  || timing < 4) )
		{
			StatData::_statData << "T," << _runTimer.GetElapsedTime() << "," << _timer.GetElapsedTime() << endl << flush;
		}
		
		delete _scheduleThread;
		delete _eventStats;

	}

	static void RunSimulation(int numEvExe)
	{
		//Steady state determined by number of events executed

		_schDec = false;
		_terminate = false;
		_eventStats = DBG_NEW EventStat(); // Statistics reset between runs

		//If not steady state
		if (!_steady)
		{
			_simTime = 0.0;

			//SWAP EVENTS FROM SEL TO EEL if first run
			_schEventList.SwapLists(_exeEventList);
		}

		_scheduleThread = DBG_NEW thread(startScheduleThread);//Start Schedule Thread
		startExecuteThread(numEvExe); //Start Execute Thread

		_scheduleThread->join();//Join threads for termination

		if (_steady && (timing > 0 || timing < 4))
		{
			StatData::_statData << "T," << _runTimer.GetElapsedTime() << "," << _timer.GetElapsedTime() << endl << flush;
		}

		delete _scheduleThread;
		delete _eventStats;

	}

	static void ScheduleEventIn(Time delta, EventAction *ea)
	{
		//Add event to the event list and then notify the scheduling thread
		RequestLock();
		_schEventList.AddEvent(_simTime + delta, ea);
		//Do not need to release lock here because AddEvent() will
		_schDec = true;
		_schCond.notify_one();
	}

	static void ScheduleEventAt(Time time, EventAction *ea)
	{
		//Add event to the event list and then notify the scheduling thread
		RequestLock();
		_schEventList.AddEvent(time, ea);
		//Do not need to release lock here because AddEvent() will
		_schDec = true;
		_schCond.notify_one();
	}

	static void startScheduleThread()
	{
		//If termination condition is not met (3-18-2019)
		while (!_terminate) {
			//wait for trigger to start scheduling
			unique_lock<mutex> _schLock(_schMutex);
			_schCond.wait(_schLock, [] {return _schDec; });

			///Could check termination condition to skip the scheduling (and notifying) at the end of the simulation
			if (!_terminate)
			{
				RequestLock();
				while (_schEventList.HasEvent())
				{
					Event* ev = _schEventList.GetEvent();
					//Do not need to release lock here because AddEvent() will
					Time eTime = ev->_time;
					EventAction* ea = ev->_ea;
					_exeEventList.AddEvent(eTime, ea);
					delete ev;
					//Need to request lock for the next HasEvent()
					RequestLock();

				}
				ReleaseLock();
				_schDec = false;
			}
		}
	};

	static bool IsEventsSimulation() {
		return _exeEventList.HasEvent();
	}

	static void startExecuteThread(Time endTime)
	{
		Time _endTime = _simTime + endTime;
		RequestLock();
		_runTimer.StartWatch();
		Time evTime = 0.0;
		while ((_exeEventList.HasEvent() || _schEventList.HasEvent()) && (evTime <= _endTime))
		{
			//Request time of each of them
			Time executeTime, schTime;

			//Get Execute Event Time
			executeTime = _exeEventList.GetTime();

			//Get Schedule Event Time
			schTime = _schEventList.GetTime();
			Event *e;
			//Get smallest time from between the two lists [min(execute eventList, sch eventList)]
			if (_exeEventList.HasEvent() && executeTime <= schTime)
			{
				evTime = executeTime;
				
				if (evTime <= _endTime) {
					e = _exeEventList.GetEvent();
					ReleaseLock();// Need to release lock
					_simTime = e->_time;
#if timing == 1
					_timer.StartWatch();
#endif

					e->_ea->Execute();
					
#if timing == 1
					_timer.EndWatch();
#endif
					_eventStats->_exeEvents++; //Increments number of events executed
					delete e;
				}
				else{
					ReleaseLock();// Need to release lock
				}
			}

			else //if (_schEventList.HasEvent() && schTime <= executeTime)
			{
				evTime = schTime;
				ReleaseLock(); // Need to release lock
				if (evTime <= _endTime) {
					e = _schEventList.GetEvent();
					_simTime = e->_time;
#if timing == 1
					_timer.StartWatch();
#endif

					e->_ea->Execute();

#if timing == 1
					_timer.EndWatch();
#endif
					_eventStats->_exeEvents++; //Increments number of events executed
					//delete e;
				}
				else {
					ReleaseLock(); //Need to release lock
				}
			}

			RequestLock();
		}

		//Insert Timing end
		_runTimer.EndWatch();

#ifdef eventStats
		if (_steady)
		{
			StatData::_statData << "T," << _eventStats->_exeEvents << "," << _eventStats->_maxExeQ << "," << _eventStats->_minExeQ << "," << _eventStats->_avgExeQ << "," << _eventStats->_maxSchQ << "," << _eventStats->_minSchQ << "," << _eventStats->_avgSchQ << "," << _eventStats->_totalMaxQ << "," << _eventStats->_totalMinQ << "," << _eventStats->_totalAvgQ << endl << flush;
		}

#endif

		ReleaseLock();
		_terminate = true;
		_schDec = true;
		_schCond.notify_one();
	};

	static void startExecuteThread(int numEvExe)
	{
		RequestLock();
		_runTimer.StartWatch();

		while ((_exeEventList.HasEvent() || _schEventList.HasEvent()) && (_eventStats->_exeEvents < numEvExe))
		{
			//Request time of each of them
			Time executeTime, schTime;

			//Get Execute Event Time
			executeTime = _exeEventList.GetTime();

			//Get Schedule Event Time
			schTime = _schEventList.GetTime();
			Event *e;

			//Get smallest time from between the two lists [min(execute eventList, sch eventList)]
			if (_exeEventList.HasEvent() && executeTime <= schTime)
			{
				e = _exeEventList.GetEvent();
			}

			else //if (_schEventList.HasEvent() && schTime <= executeTime)
			{
				e = _schEventList.GetEvent();
			}

			//Need to release lock
			ReleaseLock();
			_simTime = e->_time;

#if timing == 1
			_timer.StartWatch();
#endif

			e->_ea->Execute();

#if timing == 1
			_timer.EndWatch();
#endif
			_eventStats->_exeEvents++; //Increments number of events executed
			//delete e;


			RequestLock();
		}
		//Insert Timing end
		_runTimer.EndWatch();

#ifdef eventStats
		if (_steady)
		{
			StatData::_statData << "T," << _eventStats->_exeEvents << "," << _eventStats->_maxExeQ << "," << _eventStats->_minExeQ << "," << _eventStats->_avgExeQ << "," << _eventStats->_maxSchQ << "," << _eventStats->_minSchQ << "," << _eventStats->_avgSchQ << "," << _eventStats->_totalMaxQ << "," << _eventStats->_totalMinQ << "," << _eventStats->_totalAvgQ << endl << flush;
		}

#endif

		ReleaseLock();
		_terminate = true;
		_schDec = true;
		_schCond.notify_one();
	};
private:
	struct Event
	{
		Event(Time time, EventAction *ea)
		{
			_time = time;
			_ea = ea;
			_nextEvent = 0;
		}
		Time _time;
		EventAction *_ea;
		Event *_nextEvent;
	};

	class EventList
	{
	public:
		EventList()
		{
			_eventHead = 0;
			_numEvents = 0;
			_ID = _nextID++;

			if (_ID == 1)
			{
				_name = "Scheduling";
			}
			else
			{
				_name = "Execute";
			}
		}

		void AddEvent(Time time, EventAction *ea)
		{
			_numEvents++;

#ifdef eventStats
			UpdateEventStats(_ID, _numEvents);
#endif
			Event *e = DBG_NEW Event(time, ea);

			if (_eventHead == NULL) {
				//event list empty
				_eventHead = e;
				ReleaseLock();
			}

			else if (time < _eventHead->_time) {
				//goes at the head of the list
				e->_nextEvent = _eventHead;
				_eventHead = e;

				ReleaseLock();
			}

			else if (_eventHead->_nextEvent == NULL)
			{
				//If the event should go after the head of the list but there is only 1 event in the list
				_eventHead->_nextEvent = e;
				ReleaseLock();
			}

			else if (_eventHead->_nextEvent != NULL && e->_time < _eventHead->_nextEvent->_time)
			{
				//If the event should go between the first and second event in the list
				e->_nextEvent = _eventHead->_nextEvent;
				_eventHead->_nextEvent = e;
				ReleaseLock();
			}

			else {
				//search for where to put the event

				Event* curr = NULL;
				curr = _eventHead->_nextEvent;
				ReleaseLock();
				while ((curr->_nextEvent != NULL) ? (e->_time >= curr->_nextEvent->_time) : false) {
					curr = curr->_nextEvent;
				}
				//ReleaseLock();

				if (curr->_nextEvent == NULL) {
					//goes at the end of the list
					curr->_nextEvent = e;
				}

				else {
					e->_nextEvent = curr->_nextEvent;
					curr->_nextEvent = e;
				}
			}

		}

		Event *GetEvent()
		{
			Event *next = _eventHead;
			_eventHead = _eventHead->_nextEvent;

			_numEvents--; //3-18-2019

			return next;
		}


		void SwapLists(EventList& EL)
		{
			int ELSize = EL.GetNumEvents();
			Event* tempHead = EL._eventHead;
			EL._eventHead = _eventHead;
			_eventHead = tempHead;

			EL._numEvents = _numEvents;
			_numEvents = ELSize;

		}

		Time GetTime()
		{
			Time nextTime = DBL_MAX;
			if (_eventHead != NULL)
			{
				nextTime = _eventHead->_time;
			}

			return nextTime;
		}

		EventAction* GetEA()
		{
			//This should only be used to check the event
			//Use GetEvent() if trying to move and event from schedule event list to execute event list.
			EventAction* nextEA = _eventHead->_ea;

			return nextEA;
		}

		bool HasEvent()
		{
			bool result = _eventHead != NULL;
			return result;
		}


		int GetNumEvents() // 3-18-2019
		{
			int num = _numEvents;
			return num;
		}

		void printList() //3-18-2019
		{
			cout << "Number of events: " << _numEvents << endl << flush;

			Event* curr = _eventHead;
			while (curr != 0)
			{
				cout << curr->_time << ", " << flush;
				curr = curr->_nextEvent;
			}

			cout << endl << flush;
		}

	private:
		Event* _eventHead;
		int _numEvents;
		int _ID;
		static int _nextID;

		string _name;
	};

	static void RequestLock()
	{
		_headlock.lock();
	};

	static void ReleaseLock()
	{
		_headlock.unlock();
	};

	static void UpdateEventStats(int id, int numEvents) //Only want to calculate when steady
	{
		if (_steady) {
			int sch = _schEventList.GetNumEvents();
			int exe = _exeEventList.GetNumEvents();
			_eventStats->_totalAvgQ = (_eventStats->_totalAvgQ*(_eventStats->_numScheduled / (_eventStats->_numScheduled + 1.0))) + ((double)(sch + exe) / (_eventStats->_numScheduled + 1.0));
			_eventStats->_numScheduled += 1.0;
			if (_eventStats->_totalMaxQ < exe + sch)
			{
				_eventStats->_totalMaxQ = exe + sch;
			}
			else if (_eventStats->_totalMinQ > exe + sch)
			{
				_eventStats->_totalMinQ = exe + sch;
			}
		}

		if (_steady && id == 0)
		{

			_eventStats->_avgExeQ = (_eventStats->_avgExeQ*(_eventStats->_numExeScheduled / (_eventStats->_numExeScheduled + 1.0))) + (double(numEvents) / (_eventStats->_numExeScheduled + 1.0));
			_eventStats->_numExeScheduled += 1.0;
			_eventStats->_exeQsum += double(numEvents);

			if (_eventStats->_maxExeQ < numEvents)
			{
				_eventStats->_maxExeQ = numEvents;
			}
			else if (_eventStats->_minExeQ > numEvents)
			{
				_eventStats->_minExeQ = numEvents;
			}
		}

		else if (_steady && id == 1)
		{

			_eventStats->_avgSchQ = (_eventStats->_avgSchQ*(_eventStats->_numSchScheduled / (_eventStats->_numSchScheduled + 1.0))) + (double(numEvents) / (_eventStats->_numSchScheduled + 1.0));
			_eventStats->_numSchScheduled += 1.0;
			_eventStats->_schQsum += double(numEvents);

			if (_eventStats->_maxSchQ < numEvents)
			{
				_eventStats->_maxSchQ = numEvents;
			}
			else if (_eventStats->_minSchQ > numEvents)
			{
				_eventStats->_minSchQ = numEvents;
			}
		}
	};


	static EventStat* _eventStats;
	static Stopwatch _runTimer;
	static Stopwatch _timer;

	static EventList _exeEventList;
	static EventList _schEventList;

	static Time _simTime;
	static bool _steady;

	static mutex _headlock;
	static thread* _scheduleThread;
	static mutex _schMutex;
	static condition_variable _schCond;
	static bool _schDec;

	//static thread* _executeThread;
	static bool _executing;
	static bool _terminate;
};

EventStat *SimulationExecutive::_eventStats;
Stopwatch SimulationExecutive::_runTimer;
Stopwatch SimulationExecutive::_timer;


int SimulationExecutive::EventList::_nextID = 0;
bool SimulationExecutive::_steady = false;
SimulationExecutive::EventList SimulationExecutive::_exeEventList;
SimulationExecutive::EventList SimulationExecutive::_schEventList;

mutex SimulationExecutive::_headlock;
Time SimulationExecutive::_simTime = 0.0;
//thread *SimulationExecutive::_executeThread;

thread *SimulationExecutive::_scheduleThread;
mutex SimulationExecutive::_schMutex;
bool SimulationExecutive::_schDec = false;
condition_variable SimulationExecutive::_schCond;

bool SimulationExecutive::_terminate = false;

//Interface between user and simulation executive
Time GetSimulationTime()
{
	return SimulationExecutive::GetSimulationTime();
}

void RunSimulation(Time endTime)
{
	SimulationExecutive::RunSimulation(endTime);
}

void RunSimulation(int numEvExe)
{
	SimulationExecutive::RunSimulation(numEvExe);
}

void ScheduleEventIn(Time delta, EventAction *ea)
{
	SimulationExecutive::ScheduleEventIn(delta, ea);
}

void ScheduleEventAt(Time time, EventAction *ea)
{
	SimulationExecutive::ScheduleEventAt(time, ea);
}

bool IsEventsSimulation() {
	return SimulationExecutive::IsEventsSimulation();
}
//Used for testing
void FlipSteady()
{
	SimulationExecutive::FlipSteady();
}