#pragma once
#include "PID.h"

class PID_Vect :
	public PID
{

public:
	PID_Vect();
	~PID_Vect();

	void reset();
	void update(const Eigen::Vector3f &ist, const Eigen::Vector3f &soll, const float &dT);
	Eigen::Vector3f getOutput();

private:
	Eigen::Vector3f errorP, errorI, errorD, errorPOld;
};

