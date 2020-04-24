#include "Variant.h"
#include <stdlib.h>
#include <math.h>

Variant::Variant()
{
}

double Variant::Uniform_0_1()
{
	return (((double)rand()) / (((double)RAND_MAX) + 1.0));
}

ExponentialRV::ExponentialRV(double mean) : Variant()
{
	_mean = mean;
}

double ExponentialRV::GetRV()
{
	return  -_mean*log(Uniform_0_1());
}

UniformRV::UniformRV(double min, double max) : Variant()
{
	_min = min;
	_max = max;
}

double UniformRV::GetRV()
{
	return ((_max - _min)*Uniform_0_1() + _min);
}

TriangularRV::TriangularRV(double min, double expected, double max) : Variant()
{
	a = min;
	c = expected;
	b = max;
	fc = (c - a) / (b - a);
	term1 = (b - a)*(c - a);
	term2 = (b - a)*(b - c);
}

double TriangularRV::GetRV()
{
	double u = Uniform_0_1();
	double x;
	if (u < fc)
		x = a + sqrt(u*term1);
	else
		x = b - sqrt((1 - u)*term2);
	return x;
}

NormalRV::NormalRV(double mean, double stdev)
{
	_mean = mean;
	_stdev = stdev;
	_sqrtValue = 2 * sqrt(2.0 / 3.1415926535);
}

#define TWO_PI 6.2831853071795864769252866

double NormalRV::GetRV()
{
	// http://www.hrpub.org/download/20140305/MS7-13401470.pdf
	double RNG = Uniform_0_1();
	return (_mean - log(RNG/(1-RNG))*_stdev/_sqrtValue);
}

PoissonRV::PoissonRV(double mean)
{
	_mean = mean;
}

double PoissonRV::GetRV()
{
	double sum = 0.0;
	int i;
	for (i = -1; sum <= _mean; i++) {
		sum += -log(Uniform_0_1());
	}
	return (double)i;
}

ConstantRV::ConstantRV(double mean)
{
	_mean = mean;
}

double ConstantRV::GetRV()
{
	return _mean;
}

WeibullRV::WeibullRV(double scale, double shape)
{
	_scale = scale;
	_shape = shape;
}

double WeibullRV::GetRV()
{
	return(_scale*pow(-log(Uniform_0_1()), 1.0 / _shape));
}

ErlangRV::ErlangRV(int scale, double shape)
{
	_scale = scale;
	_shape = shape;
}

double ErlangRV::GetRV()
{
	double rv = 1.0;
	for (int i = 1; i <= _scale; i++) {
		rv *= Uniform_0_1();
	}
	return -_shape*log(rv);
}
