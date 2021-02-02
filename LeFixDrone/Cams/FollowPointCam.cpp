#include "FollowPointCam.h"

FollowPointCam::FollowPointCam(float d)
	: distance(d)
{
	endPos = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	slerpPos = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
}

FollowPointCam::~FollowPointCam()
{
}

void FollowPointCam::setDistance(float d)
{
	distance = d;
}

void FollowPointCam::update(const Eigen::Vector3f& point)
{
	float dT = MISC::GET_FRAME_TIME();

	endPos = (endPos - point).normalized()*distance + point;

	vel = (endPos - slerpPos)*5.0f;

	slerpPos += vel*dT;

	CAM_X::SET_CAM_COORD(cam, slerpPos);
	CAM_X::POINT_CAM_AT_COORD(cam, point);
}

Eigen::Vector3f FollowPointCam::getVel()
{
	return vel;
}

Eigen::Vector3f FollowPointCam::getPos()
{
	return slerpPos;
}