#include "PID_Vect.h"



PID_Vect::PID_Vect()
	: PID()
{
	reset();
}


PID_Vect::~PID_Vect()
{
}

void PID_Vect::reset()
{
	errorP = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	errorI = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
}

void PID_Vect::update(const Eigen::Vector3f &ist, const Eigen::Vector3f &soll, const float &dT)
{
	if (dT == 0.0f) return;

	errorPOld = errorP;	//Needed for derivative

	errorP = soll - ist;							//Proportional
	errorI = errorI + errorP;						//Integral
	Eigen::Vector3f preErrorD = (errorP - errorPOld) / dT;	//Differential

	errorD = 0.2f * (errorD + 4.0f * preErrorD); //Lags
}

Eigen::Vector3f PID_Vect::getOutput()
{
	//Output
	return errorP * kP + errorI * kI + errorD * kD;
}