#include "IndieCam.h"



IndieCam::IndieCam()
{
	cam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
}


IndieCam::~IndieCam()
{
	CAM::DESTROY_CAM(cam, TRUE);
}

void IndieCam::setActive()
{
	CAM::SET_CAM_ACTIVE(cam, true); CAM::RENDER_SCRIPT_CAMS(TRUE, 0, 3000, false, false, 0);
}

void IndieCam::update()
{

}