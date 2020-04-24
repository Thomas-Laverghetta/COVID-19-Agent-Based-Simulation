#pragma once
#include <cmath>
#include <float.h>
// Parameter Object to calculate probability of state transition
class Parameter {
public:
	Parameter(unsigned int numPara) : _numParameters(numPara) {}
	unsigned int GetNumParameters() { return _numParameters; }
	virtual ~Parameter() {}
protected:
	unsigned int _numParameters;
};
class ProbabilityParameters : public Parameter {
public:
	ProbabilityParameters(unsigned int numParameters) : Parameter{ numParameters } {
		_param = new float[numParameters];				// Para 1
		_index = 0;
	}
	void AddParam(float param) {
		_param[_index] = param;
		_index++;
	}
	void resetIndex() {
		_index = 0;
	}
	unsigned int size() { return _numParameters; }
	float operator[](unsigned int index) {
		if (index <= index)
			return _param[index];
		else
			return -1;
	}
	~ProbabilityParameters() {
		delete[] _param;
	}
private:
	float* _param;
	unsigned int _index;
};

class Probability {
public:
	virtual float GetProb() = 0;
	virtual float GetProb(float x) = 0;
	virtual Probability* New(ProbabilityParameters& p) = 0;
};
// Application Probability Variants
class ConstantProb : public Probability {
public:
	ConstantProb(float probability) : _probability(probability) {}

	float GetProb() {
		return _probability;
	}
	float GetProb(float x) {
		return _probability;
	}

	Probability* New(ProbabilityParameters & p) {
		return new ConstantProb(p[0]);
	}
private:
	float _probability;
};

class DistanceProb : public Probability {
public:
	DistanceProb(float distanceRate) : _distanceRate(distanceRate) {}
	float GetProb() {
		return _distanceRate / 100;
	}
	float GetProb(float x) {
		return exp(-_distanceRate * x) - exp(-FLT_MAX) - 0.05;
	}
	Probability* New(ProbabilityParameters& p) {
		return new DistanceProb(p[0]);
	}
private:
	float _distanceRate;
};

