#include "Clone.h"


Clone::Clone(Ped original)
{
	ped = PED::CLONE_PED(original, 0.0f, FALSE, TRUE);

	//Move Clone to original
	//ENTITY_X::SET_ENTITY_COORDS(ped, ENTITY_X::GET_ENTITY_COORDS(original) + Vector3f(0.0f, 0.0f, -0.45f));
	//ENTITY_X::SET_ENTITY_QUATERNION(ped, ENTITY_X::GET_ENTITY_QUATERNION(original));
	//ENTITY::SET_ENTITY_HEADING(ped, ENTITY::GET_ENTITY_HEADING(original));

	//Blip
	blip = UI_X::_BLIP_ADD_FOR_ENTITY(-89429847, ped);

	cam1 = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
	CAM::SET_CAM_NEAR_CLIP(cam1, 0.6f);
	//Head 0x796e //INDEV
	CAM_X::ATTACH_CAM_TO_ENTITY(cam1, ped, Vector3f(0.0f, 0.0f, 0.8f), true);
}

Clone::~Clone()
{
	//Delete Blip
	RADAR::REMOVE_BLIP(&blip);

	//Delete Controller
	ENTITY::DELETE_ENTITY(&objController);

	//Delete Ped
	ENTITY::SET_ENTITY_COLLISION(ped, FALSE, FALSE);
	ENTITY::DELETE_ENTITY(&ped);

	

	//Delete cam
	CAM::DESTROY_CAM(cam1, true);
}

Vector3f Clone::getPosition()
{
	return ENTITY_X::GET_ENTITY_COORDS(ped);
}

Quaternionf Clone::getQuaternion()
{
	return ENTITY_X::GET_ENTITY_QUATERNION(ped);
}

void Clone::setPedFreeze(bool freeze)
{
	ENTITY::FREEZE_ENTITY_POSITION(ped, freeze?TRUE:FALSE);
}

void Clone::refreshSettingsDynamic()
{

}

void Clone::refreshCamMode()
{
	switch (Settings::camMode)
	{
	case LeFix::camModeC1: CAM::SET_CAM_ACTIVE(cam1, true); CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, false, false, 0); break;
	default: break;
	}
}

void Clone::update(Vector3f lookAt)
{
	if (CAM::IS_CAM_RENDERING(cam1))
	{
		CAM_X::POINT_CAM_AT_COORD(cam1, lookAt);

		Vector3f position = Convert(CAM::GET_CAM_COORD(cam1));

		CAM::SET_CAM_FOV(cam1, calculateFOV((lookAt - position).norm()));

		Quaternionf rot = CAM_X::GET_CAM_QUATERNION(cam1);

		//See updateCamera
		AudioHandler::setListener(position, Vector3f(0.0f, 0.0f, 0.0f), rot); //INDEV camera turns
	}
}

float Clone::calculateFOV(float distance)
{
	static const float closeDist = 2.0f;	//Close Fov @ 2 meters distance

	/*
	Constant Object Size: c
	Close Distance: dist
	Close Field of View: alpha
	Arbitrary Distance: dist'
	Arbitrary Field of View: alpha'

	c = dist*tan(alpha/2) = dist'*tan(alpha'/2)
	->
	alpha' = 2*arctan(dist/dist'*tan(alpha/2))

	infFOV is (lower) hardlimit for FOV
	*/

	return RadToDeg( 2.0f*atan(2.0f/distance*tanf(DegToRad(float(Settings::camClone1CloseFOV - Settings::camClone1FarFOV))/2.0f)) ) + float(Settings::camClone1FarFOV);
}