#pragma once
#include <chrono>

namespace stopwatch {
	typedef std::chrono::high_resolution_clock::time_point t_point;
	typedef std::chrono::high_resolution_clock Clock;

	class Stopwatch
	{
	public:

		Stopwatch()
		{
			_total = 0.0;
		}

		void StartWatch()
		{
			_start = Clock::now();
		}

		void Lap()
		{
			_end = Clock::now();
			std::chrono::duration<double> diff = _end - _start;
			_total = _total + diff.count();
			_start = _end;
		}

		void EndWatch()
		{
			_end = Clock::now();
			std::chrono::duration<double> diff = _end - _start;
			_total = _total + diff.count();
		}

		void Reset()
		{
			_total = 0.0;
		}

		double GetElapsedTime()
		{
			return _total;
		}
	private:
		t_point _start;
		t_point _end;
		double _total;

	};
}
