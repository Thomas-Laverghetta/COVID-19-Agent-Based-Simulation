#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H
#include <cmath>
#include <float.h>

class Probability {
public:
	virtual float GetProb() = 0;
	virtual float GetProb(float x) = 0;
};

// Application Probability Distributions
class ConstantProbability : public Probability {
public:
	ConstantProbability(double probability) : _probability(probability) {}

	float GetProb() {
		return _probability;
	}
	float GetProb(float x) {
		return _probability;
	}

private:
	float _probability;
};



class Distribution
{
public:
	Distribution();
	virtual double GetRV() = 0;
protected:
	double Uniform_0_1();
};

class Exponential : public Distribution
{
public:
	Exponential(double mean);
	double GetRV();
private:
	double _mean;
};

class Uniform : public Distribution
{
public:
	Uniform(double min, double max);
	double GetRV();
private:
	double _min, _max;
};

class Triangular : public Distribution
{
public:
	Triangular(double min, double expected, double max);
	double GetRV();
private:
	double a, b, c, fc, term1, term2;
};

class Normal : public Distribution
{
public:
	Normal(double mean, double stdev);
	double GetRV();
private:
	double _sqrtValue;
	double _mean;
	double _stdev;
};

class Poisson : public Distribution
{
public:
	Poisson(double mean);
	double GetRV();
private:
	double _mean;
};

class Constant : public Distribution
{
public:
	Constant(double mean);
	double GetRV();
private:
	double _mean;
};

class Weibull : public Distribution
{
public:
	Weibull(double scale, double shape);
	double GetRV();
private:
	double _scale, _shape;
};

class Erlang : public Distribution
{
public:
	Erlang(int scale, double shape);
	double GetRV();
private:
	int _scale;
	double _shape;
};



#endif
