#include "pid.hpp"

#include <string>
#include <iostream>
#include <fstream>


//computing output
void
PID::compute(double input)
{
	double proportional = wP_ * input;
	double integral = integralSum_ + wI_ * input * period_;
	double derivative = wD_ * (input - lastInput_) / period_;

	integralSum_ = integral;
	lastInput_ = input;

	double result = proportional + integral + derivative;

	output_ = result;
	if (result > maxOutput_)
		output_ = maxOutput_;
	if (result < minOutput_)
		output_ = minOutput_;
}

double
PID::getOutput()
{
	return output_;
}
