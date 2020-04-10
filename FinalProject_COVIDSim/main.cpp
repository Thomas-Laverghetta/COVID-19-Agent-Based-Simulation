#include "Environment.h"
#include "SimObj.h"

int main() {
	InitSimulation();

	Environment test(10, 2, 6, 4, 2, 1.0, nullptr);

	RunSimulation(100);
	return 0;
}