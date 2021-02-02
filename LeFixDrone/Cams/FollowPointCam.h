#pragma once

#include "IndieCam.h"

class FollowPointCam :
	public IndieCam
{
public:
	FollowPointCam(float d);
	~FollowPointCam();

	void update(const Eigen::Vector3f& point);
	void setDistance(float d);

	Eigen::Vector3f getVel();
	Eigen::Vector3f getPos();

private:
	Eigen::Vector3f endPos, slerpPos, vel;
	float distance;
};

