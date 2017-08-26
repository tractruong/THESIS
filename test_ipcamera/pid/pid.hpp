#ifndef PID_HPP
#define PID_HPP

#include <string>

// Input for pid is error(t) = x0 - x (difference beetwen required value (x0) and current value (x))
class PID
{

public:
		// delta X, Y
		double Error;

        void compute( double input );           // Computing the output signal

        double getOutput();                     // Get the output after computing


private:


	// P, I, D weights
        const double wP_ = 1;                     
        const double wI_ = 0;                     //
        const double wD_ = 2;                     //
	//initial for PID
        double lastInput_ = 0;              // Last input, given 1 period ago
		double integralSum_ = 0;            // Integral sum

        double output_ = 0;                 // Output after computing

        const double period_ =100;                 // Refreshing period in milliseconds
	// limit output
        const double minOutput_ = -90;              // Output is limited
        const double maxOutput_ = 90;              //


};

#endif // PID_HPP