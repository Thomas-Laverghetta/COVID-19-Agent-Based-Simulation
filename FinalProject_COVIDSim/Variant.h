#ifndef Variant_H
#define Variant_H
#include "Probability.h"

class Variant
{
public:
	Variant();
	virtual double GetRV() = 0;
	virtual Variant* New(ProbabilityParameters& param) = 0;
protected:
	double Uniform_0_1();
};

class ExponentialRV : public Variant
{
public:
	ExponentialRV(double mean);
	double GetRV();
	
	Variant* New(ProbabilityParameters& param) {
		return new ExponentialRV(param[0]);
	}
private:
	double _mean;
};

class UniformRV : public Variant
{
public:
	UniformRV(double min, double max);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new UniformRV(param[0], param[1]);
	}
private:
	double _min, _max;
};

class TriangularRV : public Variant
{
public:
	TriangularRV(double min, double expected, double max);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new TriangularRV(param[0], param[1], param[2]);
	}
private:
	double a, b, c, fc, term1, term2;
};

class NormalRV : public Variant
{
public:
	NormalRV(double mean, double stdev);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new NormalRV(param[0], param[1]);
	}
private:
	double _sqrtValue;
	double _mean;
	double _stdev;
};

class PoissonRV : public Variant
{
public:
	PoissonRV(double mean);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new PoissonRV(param[0]);
	}
private:
	double _mean;
};

class ConstantRV : public Variant
{
public:
	ConstantRV(double mean);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new ConstantRV(param[0]);
	}
private:
	double _mean;
};

class WeibullRV : public Variant
{
public:
	WeibullRV(double scale, double shape);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new WeibullRV(param[0], param[1]);
	}
private:
	double _scale, _shape;
};

class ErlangRV : public Variant
{
public:
	ErlangRV(int scale, double shape);
	double GetRV();
	Variant* New(ProbabilityParameters& param) {
		return new ErlangRV(param[0], param[1]);
	}
private:
	int _scale;
	double _shape;
};
#endif
