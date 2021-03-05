#include "script.h"

#include "inc\main.h"

#include "AudioHandler.h"
#include "Clone.h"
#include "TimeCycleManager.h"
#include "Drone\Drone.h"
#include "Input\Gamepad.h"

#include "Menu.h"

#include "Utils\INIutils.hpp"

#include "Graphics\CurvePlot.h"
#include "Graphics\StickPlot.h"
#include "nativesExtended.h"

Menu menu;

Drone *drone = nullptr;
Clone *clone = nullptr;

Gamepad gamepad;

LeFix::eEXIT currentExitCode = LeFix::exitNo;

StickPlot stickPlot(&gamepad);
CurvePlot curvePlot(&gamepad);

void initialize()
{
	std::string path_settings_mod = GetCurrentModulePath() + "LeFixDrone\\settings_mod.ini";
	std::string audioPath50 = GetCurrentModulePath() + "LeFixDrone\\AudioLow.wav";
	std::string audioPath80 = GetCurrentModulePath() + "LeFixDrone\\AudioHigh.wav";

	//Settings
	Settings::SetFile(path_settings_mod);
	Settings::Load();
	Settings::Save();

	//Audio
	AudioHandler::initialize();
	AudioHandler::loadFile(audioPath50.c_str());
	AudioHandler::createBuffer(LeFix::mid);
	AudioHandler::loadFile(audioPath80.c_str());
	AudioHandler::createBuffer(LeFix::hgh);

	Drone::applyDragThrust(); //Static

	stickPlot.refreshData();
	curvePlot.refreshData();
}

void ScriptMain()
{
	initialize();
	while (true) {

		update();

		if (isAbleToStartFlight() && drone == nullptr)	//Start Flight?
		{
			UI_X::DRAW_TEXT("Press \"Enter\" to start Quadcopter Flight", 0.052f + 0.15f, 0.076f, 255, 255, 255, 255, true, 0.5f);
			GRAPHICS_X::DRAW_RECT(0.052f, 0.058f, 0.3f, 0.074f, 0, 0, 0, 190);
			GRAPHICS_X::DRAW_SPRITE("generic_textures", "menu_header_1a", 0.052f, 0.058f, 0.3f, 0.074f, 0, 255, 255, 255, 255);
			if (gamepad.button_accept) startFlight();
		}
	}
}

void onMenuEnter() {
	Settings::Load();
	//float tscale = 0.05f;
	//MISC::SET_TIME_SCALE(tscale);
	//AudioHandler::setTimeScale(tscale);
}
void onMenuExit() {
	Settings::Save();
	//float tscale = 1.0f;
	//MISC::SET_TIME_SCALE(tscale);
	//AudioHandler::setTimeScale(tscale);
}

void update()
{
	//Input
	gamepad.refresh();

	//TimeCycle (ScreenEffects)
	static int count = 0;
	if (count == 20) {
		TimeCycleManager::update();
		count = 0;
	}
	count++;

	//No drone present?
	if (drone == nullptr) {

	}
	//Drone present?
	else
	{
		//Flight
		updateFlight();

		//Stickplot
		if (Settings::showStickCam) {
			stickPlot.refreshData();
			stickPlot.draw();
		}

		//Toggle Cameramode
		if (gamepad.button_cam)
		{
			Settings::camMode++;
			if (Settings::camMode > LeFix::camModeC1) Settings::camMode = 0;
			drone->applyCam();
			clone->refreshCamMode();
		}
		//CAM::SET_CAM_AFFECTS_AIMING() //TOMTOM

		//Fast Exit?
		if (
			PLAYER::IS_PLAYER_DEAD(PLAYER::PLAYER_ID()) ||
			PLAYER::IS_PLAYER_BEING_ARRESTED(PLAYER::PLAYER_ID(), TRUE) ||
			GAMEPLAY::GET_MISSION_FLAG())
		{
			endFlightQuick();
			currentExitCode = LeFix::exitNo;
		}
		else if (currentExitCode == LeFix::exitNo)
		{
			updateMenu();
			if (currentExitCode != LeFix::exitNo)
			{
				menu.CloseMenu();
				onMenuExit();
				endFlight(currentExitCode == LeFix::exitStart);
				currentExitCode = LeFix::exitNo;
			}
		}
	}
	WAIT(0);
}

void updateFlight()
{
	//Disable Buttons and activites while flying drone
	disableFlightButtons();

	//Update Drone
	drone->update(gamepad);

	//Update Clone
	clone->update(drone->getPosition());

	//Update Ped (load world)
	ENTITY_X::SET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), drone->getPosition());
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), FALSE);
	WEAPON::SET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), 0xA2719263, TRUE, FALSE, FALSE, FALSE); //unarmed
}

void updateMenu()
{
	char* stringsCam[] = { "Drone 1st Person", "Drone 3rd Person", "Drone Follow", "Player Dynamic" };

	//Apply bools
	bool d, c, b, m, s, g, v;

	menu.ButtonMonitoring();
	switch (menu.getSubmenu())
	{
	case Main_Menu:
		menu.addTitle("Quadcopter");
		menu.addHeader("v1.0");
		menu.addFloatOption("Volume", &Settings::audioVolume, 0.0f, 1.0f, 0.1f, "Global volume for all mod-related sounds.");
		menu.addSubmenuOption("Camera", cameramenu, "Settings for all camera modes.");
		menu.addSubmenuOption("Control", controlmenu, "Settings for conversion of gamepad output to drone\ninput.");
		menu.addSubmenuOption("Drone", dronemenu, "Drone specific settings.");
		menu.addSubmenuOption("Gamepad", gamepadmenu, "Gamepad/input device specific settings.");
		menu.addSubmenuOption("Physics", physxmenu, "General physics settings.");
		menu.addSubmenuOption("Visual", visualmenu, "Visibility, light trails...");
		menu.addSubmenuOption("Exit", exitmenu, "Exit drone mode here.");

		break;
	case cameramenu:
		//Title
		menu.addTitle("Camera");

		//Apply bools
		d = false;
		c = false;

		//Camera Mode
		if (menu.addStringOption("Cam Mode", stringsCam[Settings::camMode], &Settings::camMode, ARRAYSIZE(stringsCam) - 1, "Choose your camera mode and adjust its specific\nsettings."))
		{
			switch (Settings::camMode)
			{
			case LeFix::camModeD1:
			case LeFix::camModeD3:
			case LeFix::camModeDF:
				d = true;
				break;
			case LeFix::camModeC1:
				c = true;
				break;
			}
		}

		//Specific Camera Settings
		switch (Settings::camMode)
		{
			case LeFix::camModeD1:
				d = d || menu.addIntOption("Field of View", &Settings::camDrone1FOV, 60, 120);
				d = d || menu.addIntOption("Camera Tilt", &Settings::camDrone1Tilt, -90, 90, 1, "To compensate the attack angle and better match\nthe actual flight direction. For racing around 30.");
				break;
			case LeFix::camModeD3:
				d = d || menu.addIntOption("Field of View", &Settings::camDrone3FOV, 60, 120);
				d = d || menu.addIntOption("Camera Tilt", &Settings::camDrone3Tilt, -90, 90, 1, "To compensate the attack angle and better match\nthe actual flight direction. For racing around 30.");
				d = d || menu.addFloatOption("Camera Y Position", &Settings::camDrone3YPos, -5.0f, 0.0f, 0.05f);
				d = d || menu.addFloatOption("amera Z Position", &Settings::camDrone3ZPos, -2.0f, 2.0f, 0.05f);
				break;
			case LeFix::camModeDF:
				break;
			case LeFix::camModeC1:
				menu.addIntOption("Field of View [Close]", &Settings::camClone1CloseFOV, 45, 120, 1, "The FOV is dynamic. Sets the FOV at a reference\ndistance of 2 meters.");
				menu.addIntOption("Field of View [Far]", &Settings::camClone1FarFOV, 0, 45, 1, "The FOV is dynamic. Sets the FOV at an infinite\ndistance.");
				break;
		}

		//Apply Changes
		if(d) drone->applyCam();
		if(c) clone->refreshCamMode();

		break;
	case controlmenu:
		//Title
		menu.addTitle("Control");

		//Apply bools
		b = false;

		//Options
		b = b || menu.addFloatOption("RC Rate PR", &Settings::contRcRatePR, 0.1f, 3.0f, 0.1f, "Linear factor for Pitch and Roll.");
		b = b || menu.addFloatOption("RC Rate Y", &Settings::contRcRateY, 0.1f, 3.0f, 0.1f, "Linear factor for Yaw.");
		b = b || menu.addFloatOption("Expo PR", &Settings::contExpoPR, 0.0f, 1.0f, 0.02f, "Vary linear and cubic portion of curve which affects\nsmall inputs but doesn't the change max rotation\nvelocity.");
		b = b || menu.addFloatOption("Expo Y", &Settings::contExpoY, 0.0f, 1.0f, 0.02f, "Vary linear and cubic portion of curve which affects\nsmall inputs but doesn't the change max rotation\nvelocity.");
		b = b || menu.addFloatOption("Rate P(itch)", &Settings::contRateP, 0.0f, 0.98f, 0.02f, "Greatly increases maximum rotation velocity without\nchanging curve at low inputs alot. Also known as\nsuper expo.");
		b = b || menu.addFloatOption("Rate R(oll)", &Settings::contRateR, 0.0f, 0.98f, 0.02f, "Greatly increases maximum rotation velocity without\nchanging curve at low inputs alot. Also known as\nsuper expo.");
		b = b || menu.addFloatOption("Rate Y(aw)", &Settings::contRateY, 0.0f, 0.98f, 0.02f, "Greatly increases maximum rotation velocity without\nchanging curve at low inputs alot. Also known as\nsuper expo.");

		//Apply changes
		if(b) curvePlot.refreshData();

		//Everytime
		curvePlot.draw();

		break;
	case dronemenu:
		//Title
		menu.addTitle("Drone");

		//Apply bools
		m = false;
		s = false;
		c = false;

		//Options
		m = m || menu.addFloatOption("Mass", &Settings::droneMass, 0.1f, 10.0f, 0.1f, "Drone mass");
		s = s || menu.addFloatOption("Max rel. Load", &Settings::droneMaxRelLoad, 0.0f, 5.0f, 0.1f, "Maximum extra load the drone is capable to carry.");
		s = s || menu.addFloatOption("Max Velocity", &Settings::droneMaxVel, 10.0f, 200.0f, 1.0f, "Maximum horizontal velocity the drone can achieve.\nImplicitly determines the drag coefficient.");
		menu.addBoolOption("3D Flying", &Settings::drone3DFly, "Enables downward/reverse thrust.");
		menu.addBoolOption("No Thrust Stick Down", &Settings::droneNoThrustDown, "No thrust stick position is stick down.");
		//c = c || menu.addBoolOption("Acro Mode", &Settings::droneAcroMode, "Enables direct control mode especially for racing.");
		
		//Apply Changes
		if (m) drone->applyCollider();
		if (s) Drone::applyDragThrust();
		if (c) drone->applyController();

		break;
	case gamepadmenu:
		//Title
		menu.addTitle("Gamepad");

		//Options
		menu.addBoolOption("Vibration", &Settings::gamepadVib, "Toggle gamepad vibration.\n(Heavy collisions)");
		menu.addBoolOption("Using inverted cam [RDR2]", &Settings::gamepadInvPitch, "Enabling inverted camera in the RDR2 options will\ninvert the pitch input, this setting will invert it again.");
		break;
	case physxmenu:
		//Title
		menu.addTitle("Physics");

		//Options
		g = menu.addFloatOption("Gravity Scale", &Settings::physxGScale, 0.5f, 2.0f, 0.1f, "Simple gravity multiplier. For fast outdoor flying >1\nis probably more fun and for indoor flying <1 is\nprobably easier.");
		c = menu.addBoolOption("Collision", &Settings::physxColl, "Toggle drone collision.");
		//menu.addBoolOption("Use PID", &Settings::pidEnable, "The desired rotation of the drone is achieved by a PID\nController in real world. When disabled the rotation\ngets set, undermines physics.");

		//Apply Changes
		if (g) Drone::applyDragThrust();
		if (g || c)	drone->applyCollider();

		break;
	case visualmenu:
		//Title
		menu.addTitle("Visual");

		//Apply Bools
		v = false;

		//Options
		menu.addBoolOption("Sticks", &Settings::showStickCam, "Prints the stick position on the bottom of the screen.\nIntended for screen capture.");
		//v = v || menu.addBoolOption("Trails", &Settings::showTrails, "Adds some particle effects at the prop positions.");
		v = v || menu.addBoolOption("Collision Box Visible", &Settings::showCollider, "Visibility of the collision box model, which is does all\nthe physics.");
		v = v || menu.addBoolOption("Drone Model Visible", &Settings::showModel, "Visibility of the drone model, which is attached to\nthe collsion box.");
		
		//Apply
		if (v) drone->applyVisual();

		break;
	case exitmenu:
		//Title
		menu.addTitle("Exit");

		//Options
		if (menu.addOption("Go back to player", "Exit drone mode and go back to start location."))
		{
			currentExitCode = LeFix::exitStart;
		}
		if (menu.addOption("Teleport player here", "Exit drone mode but teleport player to current\nlocation."))
		{
			currentExitCode = LeFix::exitHere;
		}

		break;
	}

	menu.resetVars();
}

bool isAbleToStartFlight()
{
	return ( ENTITY::GET_ENTITY_SPEED(PLAYER::PLAYER_PED_ID()) < 0.2f &&			//Not moving
		!ENTITY::IS_ENTITY_IN_WATER(PLAYER::PLAYER_PED_ID()) &&						//Not swimming
		CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_LT")) &&
		PED::IS_PED_ON_FOOT(PLAYER::PLAYER_PED_ID()) &&								//On Foot
		!GAMEPLAY::GET_MISSION_FLAG() );											//No active mission
}

void startFlight()
{
	//Fade out
	CAM::DO_SCREEN_FADE_OUT(400);
	WAIT_LONG(500);

	//Get reference
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	//Change PlayerPed
	ENTITY::SET_ENTITY_COLLISION(playerPed, FALSE, FALSE);
	//NOT WORK
	//RADAR::SET_BLIP_DISPLAY(RADAR::GET_MAIN_PLAYER_BLIP_ID(), 3);

	//Clone PlayerPed
	clone = new Clone(playerPed);

	//ADDITIONAL
	ENTITY::FREEZE_ENTITY_POSITION(playerPed, TRUE);
	PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), TRUE);

	Quaternionf pedRot = ENTITY_X::GET_ENTITY_QUATERNION(playerPed);
	Vector3f pedPos = ENTITY_X::GET_ENTITY_COORDS(playerPed);

	Vector3f dronePos = pedPos + Vector3f(0.0f, 0.0f, -0.5f) + pedRot._transformVector(Vector3f(0.0, 1.0f, 0.0f)); //lower in front of player
	Vector3f droneVel = Vector3f(0.0f, 0.0f, 0.0f);
	Quaternionf droneRot = pedRot;

	drone = new Drone(dronePos, droneVel, droneRot);

	clone->refreshCamMode();

	//Fade in
	WAIT_LONG(300);
	CAM::DO_SCREEN_FADE_IN(400);
	//NOT WORK
	//TimeCycleManager::setTimecycleFadeOut("NG_filmic21", 7.0f);

	clone->setPedFreeze(true);
}
void endFlight(bool goBack)
{
	//Get reference
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	//Audio
	//NOT WORK
	//AUDIO::_PLAY_AMBIENT_SPEECH1(PLAYER::PLAYER_PED_ID(), "ROLLERCOASTER_CHAT_NORMAL");

	//Fade out
	CAM::DO_SCREEN_FADE_OUT(400);
	WAIT_LONG(500);

	//Use specific loacation
	float distance;
	if (goBack)
	{
		distance = (clone->getPosition() - drone->getPosition()).norm();

		//Move PlayerPed to Clone
		ENTITY_X::SET_ENTITY_COORDS(playerPed, clone->getPosition());
		ENTITY_X::SET_ENTITY_QUATERNION(playerPed, clone->getQuaternion());
	}
	else
	{
		distance = 0.0f;
		//Teleport Player to drone (already done)
	}

	//Delete clone
	delete clone;
	clone = nullptr;

	//Change PlayerPed
	ENTITY::SET_ENTITY_COLLISION(playerPed, TRUE, FALSE);
	ENTITY::SET_ENTITY_VISIBLE(playerPed, TRUE);
	//NOT WORK
	//HUD::SET_BLIP_DISPLAY(HUD::GET_MAIN_PLAYER_BLIP_ID(), 2);

	//Additional
	RADAR::UNLOCK_MINIMAP_ANGLE();
	ENTITY::FREEZE_ENTITY_POSITION(playerPed, FALSE);
	PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), FALSE);

	delete drone;
	drone = nullptr;

	//Wait for loading world
	DWORD waitTime = 500 + DWORD(distance);
	WAIT_LONG(waitTime);
	
	//Fade in
	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, FALSE, FALSE, 0);
	CAM::DO_SCREEN_FADE_IN(400);
}
void endFlightQuick()
{
	//Close Menu
	menu.CloseMenu();

	//Delete clone
	delete clone;
	clone = nullptr;

	Ped playerPed = PLAYER::PLAYER_PED_ID();

	//Change PlayerPed
	ENTITY::SET_ENTITY_COLLISION(playerPed, TRUE, FALSE);
	ENTITY::SET_ENTITY_VISIBLE(playerPed, TRUE);
	//NOT WORK
	//HUD::SET_BLIP_DISPLAY(HUD::GET_MAIN_PLAYER_BLIP_ID(), 2);

	//Additional
	RADAR::UNLOCK_MINIMAP_ANGLE();
	ENTITY::FREEZE_ENTITY_POSITION(playerPed, FALSE);
	PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), FALSE);

	delete drone;
	drone = nullptr;

	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, FALSE, FALSE, 0);
}

void disableFlightButtons()
{
	//NOT WORK
	CONTROLS::DISABLE_CONTROL_ACTION(0, GAMEPLAY_X::JOAAT("INPUT_SELECT_WEAPON"), TRUE);
}

void WAIT_LONG(DWORD waitTime)
{
	static DWORD maxTickCount;
	maxTickCount = GetTickCount() + waitTime;
	do {
		WAIT(0);
		update();
	} while (GetTickCount() < maxTickCount);
}

