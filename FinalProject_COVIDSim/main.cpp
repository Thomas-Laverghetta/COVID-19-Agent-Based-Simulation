#include "Environment.h"
#include "SimObj.h"

int main() {
	InitSimulation();

	Environment test(1000000, 2, 600, 400, 2, 1.0, nullptr);

	RunSimulation(30);
	return 0;
}