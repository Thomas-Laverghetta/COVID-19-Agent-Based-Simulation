#include "Distribution.h"
#include <stdlib.h>
#include <math.h>

Distribution::Distribution()
{
}

double Distribution::Uniform_0_1()
{
	return (((double)rand()) / (((double)RAND_MAX) + 1.0));
}

Exponential::Exponential(double mean) : Distribution()
{
	_mean = mean;
}

double Exponential::GetRV()
{
	return  -_mean*log(Uniform_0_1());
}

Uniform::Uniform(double min, double max) : Distribution()
{
	_min = min;
	_max = max;
}

double Uniform::GetRV()
{
	return ((_max - _min)*Uniform_0_1() + _min);
}

Triangular::Triangular(double min, double expected, double max) : Distribution()
{
	a = min;
	c = expected;
	b = max;
	fc = (c - a) / (b - a);
	term1 = (b - a)*(c - a);
	term2 = (b - a)*(b - c);
}

double Triangular::GetRV()
{
	double u = Uniform_0_1();
	double x;
	if (u < fc)
		x = a + sqrt(u*term1);
	else
		x = b - sqrt((1 - u)*term2);
	return x;
}

Normal::Normal(double mean, double stdev)
{
	_mean = mean;
	_stdev = stdev;
	_sqrtValue = 2 * sqrt(2.0 / 3.1415926535);
}

#define TWO_PI 6.2831853071795864769252866

double Normal::GetRV()
{
	// http://www.hrpub.org/download/20140305/MS7-13401470.pdf
	double RNG = Uniform_0_1();
	return (_mean - log(RNG/(1-RNG))*_stdev/_sqrtValue);
}

Poisson::Poisson(double mean)
{
	_mean = mean;
}

double Poisson::GetRV()
{
	double sum = 0.0;
	int i;
	for (i = -1; sum <= _mean; i++) {
		sum += -log(Uniform_0_1());
	}
	return (double)i;
}

Constant::Constant(double mean)
{
	_mean = mean;
}

double Constant::GetRV()
{
	return _mean;
}

Weibull::Weibull(double scale, double shape)
{
	_scale = scale;
	_shape = shape;
}

double Weibull::GetRV()
{
	return(_scale*pow(-log(Uniform_0_1()), 1.0 / _shape));
}

Erlang::Erlang(int scale, double shape)
{
	_scale = scale;
	_shape = shape;
}

double Erlang::GetRV()
{
	double rv = 1.0;
	for (int i = 1; i <= _scale; i++) {
		rv *= Uniform_0_1();
	}
	return -_shape*log(rv);
}
