#include "Drone.h"

#include "..\Settings.h"
#include "..\TimeCycleManager.h"
#include "..\Steuerung.h"

using namespace LeFix;

//-------------------STATIC VARIABLES-----------------------

const float Drone::gravity(20.0f); //Force has factor 0.5f

const Vector3f Drone::camDefaultOffsetLocal(0.0f, 0.05f, 0.00f); //Default Camera Position
const Quaternionf Drone::colliderRotLocal = Quaternionf(AngleAxisf(DegToRad(90.0f), Vector3f(0.0f, 1.0f, 0.0f))); //Rotation of collider //Indev angle 90!!
const Vector3f Drone::propPosLocal[NUM_PROP] = { Vector3f(0.5f, 0.5f, 0.0f), Vector3f(0.5f, -0.5f, 0.0f), Vector3f(-0.5f, -0.5f, 0.0f), Vector3f(-0.5f, 0.5f, 0.0f) };

float Drone::maxThrust, Drone::dragCoef; //calculated
Quaternionf Drone::cam1RotLocal, Drone::cam3RotLocal;


//--------------------STATIC METHODS-------------------------

void Drone::applyDragThrust()
{
	float g = gravity*Settings::physxGScale;
	//Dependent stats
	maxThrust = g * (1.0f + Settings::droneMaxRelLoad);
	dragCoef = sqrtf( (maxThrust*maxThrust - g*g) ) / (Settings::droneMaxVel*Settings::droneMaxVel); // sqrt(t*t-g*g)/(vel*vel)
}

//--------------------MEMBER PUBLIC---------------------------

void Drone::applyController()
{
	//Controller
	delete controller;
	if (Settings::droneAcroMode)
	{
		controller = new DroneControllerAcro();
	}
	else {
		controller = new DroneControllerLevel();
	}
	resetHistory();
}

void Drone::applyCollider()
{
	//New phyiscs params: mass and gscale
	OBJECT::SET_OBJECT_PHYSICS_PARAMS(collider, Settings::droneMass, Settings::physxGScale, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f);

	//Collision
	ENTITY::SET_ENTITY_COLLISION(collider, Settings::physxColl, TRUE);

	//LOAD NEW PHYSICS? (SET STATIC THEN DYNAMIC AGAIN)
	ENTITY::SET_ENTITY_DYNAMIC(collider, FALSE);
	ENTITY::SET_ENTITY_DYNAMIC(collider, TRUE);
	currentState.apply(); //Get velocity back
}

void Drone::applyPID()
{
	pidRot.reset();
	pidRot.kP = Settings::pidP;
	pidRot.kI = Settings::pidI;
	pidRot.kD = Settings::pidD;
}

void Drone::applyCam()
{
	//STATIC Tilt
	Drone::cam1RotLocal = Quaternionf(AngleAxisf(DegToRad((float)Settings::camDrone1Tilt), Vector3f(1.0f, 0.0f, 0.0f)));
	Drone::cam3RotLocal = Quaternionf(AngleAxisf(DegToRad((float)Settings::camDrone3Tilt), Vector3f(1.0f, 0.0f, 0.0f)));

	//FOV
	CAM::SET_CAM_FOV(cam1, (float)Settings::camDrone1FOV);
	CAM::SET_CAM_FOV(cam3, (float)Settings::camDrone3FOV);

	//Cam3 attach at new coord
	Vector3f cam3PosDrone = Vector3f(0.0f, Settings::camDrone3YPos, Settings::camDrone3ZPos) + camDefaultOffsetLocal;
	Vector3f cam3PosColld = colliderRotLocal.conjugate().toRotationMatrix() * cam3PosDrone;
	CAM_X::ATTACH_CAM_TO_ENTITY(cam3, collider, cam3PosColld, TRUE);

	switch (Settings::camMode)
	{
	case LeFix::camModeD1: CAM::SET_CAM_ACTIVE(cam1, true); CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, false, false, 0); break;
	case LeFix::camModeD3: CAM::SET_CAM_ACTIVE(cam3, true); CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, false, false, 0); break;
	case LeFix::camModeDF: camF.setActive(); break;
	default: break;
	}
}

void Drone::applyVisual()
{
	//PTFX
	setTrails(Settings::showTrails);

	//Set collider invisble before model visibility is set because model is attached to it
	ENTITY::SET_ENTITY_VISIBLE(collider, Settings::showCollider, FALSE);

	//Attached stuff (frisbees and mics) is automatically set
	ENTITY::SET_ENTITY_VISIBLE(modelCase, Settings::showModel, FALSE);
}

Drone::Drone(Vector3f pos, Vector3f vel, Quaternionf rot)
	: camF(5.0f)
{
	//p_rc_handset 2553089994 //prop_cs_power_cell 1456723945 //prop_c4_final 3028688567 //prop_microphone_02 933500565 //p_ld_frisbee_01 3024733075
	Hash colliderHash = -589090886; //Big box 2781083456

	//Load Collision Box
	DWORD timeout = GetTickCount() + 5000;
	while (!(STREAMING::HAS_MODEL_LOADED(colliderHash) && STREAMING::HAS_COLLISION_FOR_MODEL_LOADED(colliderHash)) && timeout > GetTickCount())
	{
		STREAMING::REQUEST_MODEL(colliderHash);
		STREAMING::REQUEST_COLLISION_FOR_MODEL(colliderHash);
		WAIT(0);
	}

	//Collision
	collider = OBJECT::CREATE_OBJECT(colliderHash, pos.x(), pos.y(), pos.z(), FALSE, TRUE, TRUE);
	ENTITY::SET_ENTITY_COLLISION(collider, TRUE, TRUE);
	ENTITY::SET_ENTITY_RECORDS_COLLISIONS(collider, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(collider, TRUE, FALSE);
	OBJECT::SET_OBJECT_PHYSICS_PARAMS(collider, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 4.0f, 4.0f, 0.0f, -1.0f, 20.0f, 0.3f); //-1.0 Values changed elsewhere

	//Model
	modelCase = OBJECT::CREATE_OBJECT(3028688567, 0.0f, 0.0f, 0.0f, FALSE, TRUE, FALSE);
	ENTITY::SET_ENTITY_COLLISION(modelCase, FALSE, FALSE);
	ENTITY::ATTACH_ENTITY_TO_ENTITY(modelCase, collider, 0, 0.04f, 0.0f, 0.0f, -90.0f, -90.0f, 0.0f, FALSE, TRUE, FALSE, FALSE, 0, TRUE);

	//Mics
	for (int n = 0; n < 4; n++)
	{
		modelProp[n] = OBJECT::CREATE_OBJECT(933500565, 0.0f, 0.0f, 0.0f, FALSE, TRUE, FALSE);
		ENTITY::SET_ENTITY_COLLISION(modelProp[n], FALSE, FALSE);
		float x, z, ry;
		switch (n)
		{
		case LeFix::FR: ry =  90.0f; x =  0.062f; z =  0.090f; break;
		case LeFix::BR: ry = 135.0f; x =  0.095f; z = -0.080f; break;
		case LeFix::BL: ry = 225.0f; x = -0.095f; z = -0.080f; break;
		case LeFix::FL: ry = 270.0f; x = -0.062f; z =  0.090f; break;
		}
		ENTITY::ATTACH_ENTITY_TO_ENTITY(modelProp[n], modelCase, 0, x, 0.030f, z, 0.0f, ry, 0.0f, FALSE, TRUE, FALSE, FALSE, 0, TRUE);
	}

	//Frisbees
	float rx = 90.0f;
	float ry[4] = { -90.0f, 135.0f, -135.0f, 90.0f };
	float rz = 0.0f;
	float x[4] = { 0.17f, 0.17f, -0.17f, -0.17f };
	float y = 0.012f;
	float z[4] = { 0.091f, -0.162f, -0.162f, 0.091f };

	for (int n = 0; n < 4; n++)
	{
		modelFris[n] = OBJECT::CREATE_OBJECT(3024733075, 0.0f, 0.0f, 0.0f, FALSE, TRUE, FALSE);
		ENTITY::SET_ENTITY_COLLISION(modelFris[n], FALSE, FALSE);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(modelFris[n], modelCase, 0, x[n], y, z[n], rx, ry[n], rz, FALSE, TRUE, FALSE, FALSE, 0, TRUE);
		ENTITY::SET_ENTITY_ALPHA(modelFris[n], 50, FALSE);
	}

	//Blip
	blip = HUD::ADD_BLIP_FOR_ENTITY(collider);
	HUD::SET_BLIP_SPRITE(blip, 8); // 4 dot
	HUD::SET_BLIP_ROTATION(blip, 45);

	//Cameras
	cam1 = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
	cam3 = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
	CAM::SET_CAM_NEAR_CLIP(cam1, 0.05f);
	CAM::SET_CAM_NEAR_CLIP(cam3, 0.05f);
	CAM::SET_CAM_NEAR_CLIP(camF.cam, 0.05f);
	CAM_X::ATTACH_CAM_TO_ENTITY(cam1, collider, camDefaultOffsetLocal, true);
	CAM_X::ATTACH_CAM_TO_ENTITY(cam3, collider, camDefaultOffsetLocal, true); //Should be changed in refreshDynamicSettings

	//Audio
	audio.playSources();

	//Ende
	currentState = DroneState(collider, colliderRotLocal);
	currentState.set(rot, pos, vel);

	applyCollider();
	applyController();
	applyPID();
	applyVisual();
	applyCam();
}

Drone::~Drone()
{
	delete controller;
	HUD::REMOVE_BLIP(&blip);
	audio.stopSources();
	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, FALSE, FALSE, 0);
	CAM::DESTROY_CAM(cam1, true);
	CAM::DESTROY_CAM(cam3, true);
	OBJECT::DELETE_OBJECT(&collider);
	OBJECT::DELETE_OBJECT(&modelCase);
	for (int x = 0; x < 4; x++)
	{
		OBJECT::DELETE_OBJECT(modelProp+x);
		OBJECT::DELETE_OBJECT(modelFris+x);
	}
}

Vector3f Drone::getPosition()
{
	return currentState.pos;
}
Vector3f Drone::getVelocity()
{
	return currentState.vel;
}
Quaternionf Drone::getRotation()
{
	return currentState.rot;
}

//---------------------MEMBER PRIVATE---------------------------

void Drone::update(Gamepad &gamepad)
{
	//Current State
	oldState = currentState;
	currentState.read();

	//Collsion check
	if (ENTITY::HAS_ENTITY_COLLIDED_WITH_ANYTHING(collider) || ENTITY::IS_ENTITY_IN_WATER(collider))
	{

		if ((currentState.vel - oldState.vel).squaredNorm() > (Settings::droneMaxVel*Settings::droneMaxVel/36.0f) )
		{
			if(Settings::gamepadVib) PAD::SET_PAD_SHAKE(0, 100, 200);
			//Screen effect when collision delta v is above maxVel/6
			if(Settings::camMode == camModeD1) TimeCycleManager::setTimecycleFadeOut("NG_filmic21", 0.5f);
		}
		if (gamepad.button_flip) flip();
		if (gamepad.button_unstuck) unstuck();
	}

	/*
	//Save and Load State
	if (gamepad.IsPressed(XINPUT_GAMEPAD_LEFT_SHOULDER)) {
		savedState = currentState; //INDEV
	}
	//if (gamepad.IsPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
		currentState = savedState;
		resetHistory;
	}
	*/

	//Physics
	if (controller != nullptr)
	{
		controller->update(currentState, gamepad.stick_left_x, gamepad.stick_left_y, gamepad.stick_right_x, gamepad.stick_right_y);

		updateMomentum(controller->getDesiredRot());
		updateForce(controller->getThrottle());
	}

	//Secondary Stuff
	updateCameras();
	updateMinimap();
	updateProps();
	updateAudioListener();
	updateAudioSources();
}

void Drone::flip()
{
	currentState.rot *= Quaternionf(AngleAxisf(DegToRad(180.0f), Vector3f(0.0f, 1.0f, 0.0f)));
	currentState.drot = Quaternionf(1.0f, 0.0f, 0.0f, 0.0f);

	currentState.apply();
	resetHistory();
}

void Drone::unstuck()
{
	currentState.rot = Quaternionf(1.0f, 0.0f, 0.0f, 0.0f);
	currentState.drot = Quaternionf(1.0f, 0.0f, 0.0f, 0.0f);
	currentState.pos += Vector3f(0.0f, 0.0f, 1.5f);
	currentState.vel = Vector3f(0.0f, 0.0f, 0.0f);

	currentState.apply();
	resetHistory();
}

void Drone::resetHistory()
{
	oldState = currentState;
	controller->reset(currentState);

	pidRot.reset();
	slerpRot = currentState.rot;
}

void Drone::setTrails(bool doEnable)
{
	static int ptfx[NUM_PROP][2];
	static bool isEnabled = false;
	if (doEnable == isEnabled) return; //Already done

	if (doEnable)
	{
		//"scr_tennis_ball_trail" from asset "scr_minigametennis"
		//"proj_grenade_trail" from asset "scr_agencyheistb"

		char asset[] = "scr_minigametennis";
		char effect[] = "scr_tennis_ball_trail";

		//Load PTFX
		DWORD timeout = GetTickCount() + 3000;
		while (!STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(asset) && timeout > GetTickCount())
		{
			STREAMING::REQUEST_NAMED_PTFX_ASSET(asset);
			WAIT(0);
		}

		//PTFX 1
		if (STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(asset)) {
			for (int n = 0; n < NUM_PROP; n++)
			{
				GRAPHICS::USE_PARTICLE_FX_ASSET(asset);
				ptfx[n][0] = GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(effect, modelFris[n], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.6f, FALSE, FALSE, FALSE);
				GRAPHICS::SET_PARTICLE_FX_LOOPED_COLOUR(ptfx[n][0], 1.0f, 0.5f, 0.0f, FALSE);
			}

			//Wait for ptfx pulse
			timeout = GetTickCount() + 500;
			while (timeout > GetTickCount()) WAIT(0);

			//Load PTFX (again?)
			timeout = GetTickCount() + 100;
			while (!STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(asset) && timeout > GetTickCount())
			{
				STREAMING::REQUEST_NAMED_PTFX_ASSET(asset);
				WAIT(0);
			}

			//PTFX 2
			if (STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(asset)) {
				for (int n = 0; n < NUM_PROP; n++)
				{
					GRAPHICS::USE_PARTICLE_FX_ASSET(asset);
					ptfx[n][1] = GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(effect, modelFris[n], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.6f, FALSE, FALSE, FALSE);
					GRAPHICS::SET_PARTICLE_FX_LOOPED_COLOUR(ptfx[n][1], 1.0f, 0.5f, 0.0f, FALSE);
				}
			}
		}
		else
		{
			showSubtitle("Could not load ptfx asset. Timeout after 3s.", 1000);
		}
	}
	else
	{
		// STOP / DISABLE
		for (int n = 0; n < NUM_PROP; n++)
		{
			GRAPHICS::STOP_PARTICLE_FX_LOOPED(ptfx[n][0], TRUE);
			GRAPHICS::STOP_PARTICLE_FX_LOOPED(ptfx[n][1], TRUE);
		}
	}

	isEnabled = doEnable;
}

void Drone::updateMomentum(const Quaternionf &desiredRot)
{
	if (Settings::pidEnable)
	{
		slerpRot = slerpRot.slerp(MISC::GET_FRAME_TIME() * 25.0f, desiredRot); //25 Frames Slerp desiredRot, should kill overrotation of pid

		//Use PID
		pidRot.update(currentState.rot, slerpRot, MISC::GET_FRAME_TIME());
		Vector3f momentum = pidRot.getOutput();

		//Apply PID Output momentum
		momentum = colliderRotLocal.conjugate()._transformVector(momentum); //To collider body-fixed coordinates

		ENTITY_X::APPLY_MOMENTUM_TO_ENTITY(collider, momentum, 0, false, true); //Apply

		//For Prop Sound
		Vector3f temp = 0.2f * momentum / (Settings::pidP + Settings::pidD);
		appliedRelMom = 0.8f*appliedRelMom + 0.2f*temp;
	}
	else
	{
		//Just take desired Rotation
		ENTITY_X::SET_ENTITY_QUATERNION(collider, desiredRot * colliderRotLocal);
		currentState.rot = desiredRot; //Does change immediately (for cameras and audio sources)

		//For Prop Sound
		AngleAxisf temp1 = AngleAxisf(currentState.drot);	//Delta Rotation As Angle Axis
		Vector3f temp2 = 0.2f * temp1.angle() * temp1.axis() / currentState.deltaTime;
		appliedRelMom = 0.8f*appliedRelMom + 0.2f*temp2;
	}
}

void Drone::updateForce(const float &contThrottle)
{
	Vector3f dragGlobal = currentState.vel*(-currentState.vel.norm()*dragCoef);
	Vector3f thrustGlobal= currentState.rot._transformVector(Vector3f(0.0f, 0.0f, contThrottle*maxThrust));

	ENTITY_X::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(collider, dragGlobal, false, false, true);
	ENTITY_X::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(collider, thrustGlobal, false, false, true);

	appliedThrottle = contThrottle;
}

void Drone::updateProps()
{
	//clockwise
	prop[LeFix::FR] = abs(appliedThrottle) + appliedRelMom.x() -appliedRelMom.y() + appliedRelMom.z(); //Front Right
	prop[LeFix::BR] = abs(appliedThrottle) - appliedRelMom.x() -appliedRelMom.y() - appliedRelMom.z(); //Rear  Right
	prop[LeFix::BL] = abs(appliedThrottle) - appliedRelMom.x() +appliedRelMom.y() + appliedRelMom.z(); //Rear  Left
	prop[LeFix::FL] = abs(appliedThrottle) + appliedRelMom.x() +appliedRelMom.y() - appliedRelMom.z(); //Front Left

	//Get Maximum
	float max = 0.0f;
	float min = 1.0f;
	for (int p = 0; p < NUM_PROP; p++) {
		if (prop[p] > max) max = prop[p];
		if (prop[p] < min) min = prop[p];
	}
	//Minimum = 0.0
	if (min < 0.0f) {
		for (int p = 0; p < NUM_PROP; p++) {
			prop[p] += -min;
		}
		max += -min;
	}
	//Maximum = 1.0;
	if (max > 1.0f) {
		for (int p = 0; p < NUM_PROP; p++) {
			prop[p] /= max;
		}
	}

}

void Drone::updateCameras()
{
	//New values, also used for listener
	cam1RotGlobal = currentState.rot * cam1RotLocal;

	cam3RotGlobal = currentState.rot * cam3RotLocal;

	//Apply
	CAM_X::SET_CAM_QUATERNION(cam1, cam1RotGlobal);
	CAM_X::SET_CAM_QUATERNION(cam3, cam3RotGlobal);

	camF.update(currentState.pos);
}

void Drone::updateMinimap()
{
	int heading;
	if (CAM::IS_CAM_RENDERING(cam1))
	{
		//use cam1
		heading = (int)getHeading(cam1RotGlobal._transformVector(Vector3f(0.0f, 1.0f, 0.0f)));
		HUD::LOCK_MINIMAP_ANGLE(heading);
	}
	else if (CAM::IS_CAM_RENDERING(cam3))
	{
		//use cam3
		heading = (int)getHeading(cam3RotGlobal._transformVector(Vector3f(0.0f, 1.0f, 0.0f)));
		HUD::LOCK_MINIMAP_ANGLE(heading);
	}
	else
	{
		//use drone (don't rotate minimap just set blip)
		heading = (int)getHeading(currentState.rot._transformVector(Vector3f(0.0f, 1.0f, 0.0f)));
	}
	HUD::SET_BLIP_ROTATION(blip, heading + 45);
	
}

void Drone::updateAudioSources()
{
	for (int p = 0; p < NUM_PROP; p++)
	{
		//New values
		propPosGlobal[p] = currentState.pos + currentState.rot._transformVector(propPosLocal[p]);

		//Apply
		audio.setSourcePosVel(p, propPosGlobal[p], currentState.vel);
		audio.setSourceSound(p, prop[p]);
	}
}

void Drone::updateAudioListener()
{
	if (CAM::IS_CAM_RENDERING(cam1))
	{
		//use cam1
		Vector3f position = Convert(CAM::GET_CAM_COORD(cam1));

		//See updateCamera
		AudioHandler::setListener(position, currentState.vel, cam1RotGlobal);
	}
	else if (CAM::IS_CAM_RENDERING(cam3))
	{
		//use cam3
		Vector3f position = Convert(CAM::GET_CAM_COORD(cam3));

		//See updateCamera
		AudioHandler::setListener(position, currentState.vel, cam3RotGlobal);
	}
	else if (CAM::IS_CAM_RENDERING(camF.cam))
	{
		AudioHandler::setListener(camF.getPos(), camF.getVel(), CAM_X::GET_CAM_QUATERNION(camF.cam));
	}
}