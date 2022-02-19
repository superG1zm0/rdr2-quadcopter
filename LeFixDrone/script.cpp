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
#include "Graphics\GamepadAxis.h"
#include "nativesExtended.h"

Menu menu;

Drone *drone = nullptr;
Clone *clone = nullptr;

Gamepad gamepad;

LeFix::eEXIT currentExitCode = LeFix::exitNo;

StickPlot stickPlot(&gamepad);
CurvePlot curvePlot(&gamepad);
GamepadAxis gamepadAxis(&gamepad);

Any quadcopter_menu_prompt;

char* stringsCam[] = { "Drone 1st Person", "Drone 3rd Person", "Drone Follow", "Player Dynamic" };

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

	quadcopter_menu_prompt = UI_X::CREATE_PROMPT(
		GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", "Start Quadcopter Flight"),
		GAMEPLAY_X::JOAAT("INPUT_FRONTEND_ACCEPT"));
	UI_X::_UIPROMPT_SET_ENABLED(quadcopter_menu_prompt, FALSE);
	UI_X::_UIPROMPT_SET_VISIBLE(quadcopter_menu_prompt, FALSE);
}

void ScriptMain()
{
	initialize();
	while (true) {

		update();

		if (isAbleToStartFlight() && drone == nullptr)	//Start Flight?
		{
			if (!UI_X::_UIPROMPT_IS_ENABLED(quadcopter_menu_prompt))
			{
				UI_X::_UIPROMPT_SET_ENABLED(quadcopter_menu_prompt, TRUE);
				UI_X::_UIPROMPT_SET_VISIBLE(quadcopter_menu_prompt, TRUE);
			}
			if (CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_ACCEPT"))) startFlight();
		}
		else
		{
			if (UI_X::_UIPROMPT_IS_ENABLED(quadcopter_menu_prompt))
			{
				UI_X::_UIPROMPT_SET_ENABLED(quadcopter_menu_prompt, FALSE);
				UI_X::_UIPROMPT_SET_VISIBLE(quadcopter_menu_prompt, FALSE);
			}
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

		//Hide HUD and radar
		if (Settings::hideHUD) {
			UI::HIDE_HUD_AND_RADAR_THIS_FRAME();
		}

		//Toggle Cameramode
		if (gamepad.button_cam)
		{
			Settings::camMode++;
			if (Settings::camMode > LeFix::camModeC1) Settings::camMode = 0;
			drone->applyCam();
			clone->refreshCamMode();
			UILOG_X::PRINT_SUBTITLE(stringsCam[Settings::camMode]);
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
	WEAPON::SET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), GAMEPLAY_X::JOAAT("weapon_unarmed"), TRUE, FALSE, FALSE, FALSE); //unarmed
}

void updateMenu()
{
	//Apply bools
	bool d, c, b, m, s, g, v;

	menu.ButtonMonitoring();
	switch (menu.getSubmenu())
	{
	case Main_Menu:
		menu.addTitle("Quadcopter");
		menu.addHeader("v1.1");
		menu.addFloatOption("Volume", &Settings::audioVolume, 0.0f, 1.0f, 0.1f, 1, "Global volume for all mod-related sounds.");
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
				d = d || menu.addFloatOption("Camera Y Position", &Settings::camDrone3YPos, -5.0f, 0.0f, 0.05f, 2);
				d = d || menu.addFloatOption("Camera Z Position", &Settings::camDrone3ZPos, -2.0f, 2.0f, 0.05f, 2);
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
		b = b || menu.addFloatOption("RC Rate PR", &Settings::contRcRatePR, 0.1f, 3.0f, 0.1f, 1, "Linear factor for Pitch and Roll.");
		b = b || menu.addFloatOption("RC Rate Y", &Settings::contRcRateY, 0.1f, 3.0f, 0.1f, 1, "Linear factor for Yaw.");
		b = b || menu.addFloatOption("Expo PR", &Settings::contExpoPR, 0.0f, 1.0f, 0.02f, 2, "Vary linear and cubic portion of curve which affects\nsmall inputs but doesn't the change max rotation\nvelocity.");
		b = b || menu.addFloatOption("Expo Y", &Settings::contExpoY, 0.0f, 1.0f, 0.02f, 2, "Vary linear and cubic portion of curve which affects\nsmall inputs but doesn't the change max rotation\nvelocity.");
		b = b || menu.addFloatOption("Rate P(itch)", &Settings::contRateP, 0.0f, 0.98f, 0.02f, 2, "Greatly increases maximum rotation velocity without\nchanging curve at low inputs alot. Also known as\nsuper expo.");
		b = b || menu.addFloatOption("Rate R(oll)", &Settings::contRateR, 0.0f, 0.98f, 0.02f, 2, "Greatly increases maximum rotation velocity without\nchanging curve at low inputs alot. Also known as\nsuper expo.");
		b = b || menu.addFloatOption("Rate Y(aw)", &Settings::contRateY, 0.0f, 0.98f, 0.02f, 2, "Greatly increases maximum rotation velocity without\nchanging curve at low inputs alot. Also known as\nsuper expo.");

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
		m = m || menu.addFloatOption("Mass", &Settings::droneMass, 0.1f, 10.0f, 0.1f, 1, "Drone mass");
		s = s || menu.addFloatOption("Max rel. Load", &Settings::droneMaxRelLoad, 0.0f, 5.0f, 0.1f, 1, "Maximum extra load the drone is capable to carry.");
		s = s || menu.addFloatOption("Max Velocity", &Settings::droneMaxVel, 10.0f, 200.0f, 1.0f, 0, "Maximum horizontal velocity the drone can achieve.\nImplicitly determines the drag coefficient.");
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
		if (menu.addBoolOption("DirectInput", &Settings::gamepadDirectInput, "Enables DirectInput."))
		{
			if (Settings::gamepadDirectInput)
				gamepad.InitDirectInput(GetForegroundWindow());
			else
				gamepad.ReleaseDirectInput();
		}

		if (Settings::gamepadDirectInput)
		{
			menu.addHeader(gamepad.name);
			menu.addIntOption("Roll", &Settings::gamepadRollAxis, 1, 6);
			menu.addIntOption("Pitch", &Settings::gamepadPitchAxis, 1, 6);
			menu.addIntOption("Yaw", &Settings::gamepadYawAxis, 1, 6);
			menu.addIntOption("Throttle", &Settings::gamepadThrottleAxis, 1, 6);

			menu.addBoolOption("Inverted Roll", &Settings::gamepadInvRoll);
			menu.addBoolOption("Inverted Pitch", &Settings::gamepadInvPitch);
			menu.addBoolOption("Inverted Yaw", &Settings::gamepadInvYaw);
			menu.addBoolOption("Inverted Throttle", &Settings::gamepadInvThrottle);

			menu.addIntOption("Roll Deadband", &Settings::gamepadRollDeadband, 0, 1000);
			menu.addIntOption("Roll Center", &Settings::gamepadRollCenter, -1000, 1000);
			menu.addFloatOption("Roll Sensitivity", &Settings::gamepadRollSensitivity, 1.0f, 2.0f, 0.01f, 2);

			menu.addIntOption("Pitch Deadband", &Settings::gamepadPitchDeadband, 0, 1000);
			menu.addIntOption("Pitch Center", &Settings::gamepadPitchCenter, -1000, 1000);
			menu.addFloatOption("Pitch Sensitivity", &Settings::gamepadPitchSensitivity, 1.0f, 2.0f, 0.01f, 2);

			menu.addIntOption("Yaw Deadband", &Settings::gamepadYawDeadband, 0, 1000);
			menu.addIntOption("Yaw Center", &Settings::gamepadYawCenter, -1000, 1000);
			menu.addFloatOption("Yaw Sensitivity", &Settings::gamepadYawSensitivity, 1.0f, 2.0f, 0.01f, 2);

			menu.addIntOption("Throttle Center", &Settings::gamepadThrottleCenter, -1000, 1000);
			menu.addFloatOption("Throttle Sensitivity", &Settings::gamepadThrottleSensitivity, 1.0f, 2.0f, 0.01f, 2);

			menu.addIntOption("Unstuck", &Settings::gamepadUnstuckButton, 0, 32);
			menu.addIntOption("Flip", &Settings::gamepadFlipButton, 0, 32);
			menu.addIntOption("Cam", &Settings::gamepadCamButton, 0, 32);
			gamepadAxis.draw();
		}
		else
		{
			menu.addBoolOption("Vibration", &Settings::gamepadVib, "Toggle gamepad vibration.\n(Heavy collisions)");
			menu.addBoolOption("Using inverted cam [RDR2]", &Settings::gamepadInvPitch, "Enabling inverted camera in the RDR2 options will\ninvert the pitch input, this setting will invert it again.");
		}
		break;
	case physxmenu:
		//Title
		menu.addTitle("Physics");

		//Options
		g = menu.addFloatOption("Gravity Scale", &Settings::physxGScale, 0.5f, 2.0f, 0.1f, 1, "Simple gravity multiplier. For fast outdoor flying >1\nis probably more fun and for indoor flying <1 is\nprobably easier.");
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
		menu.addBoolOption("Hide HUD", &Settings::hideHUD, "Hides HUD and mini map.");
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
	return ( //ENTITY::GET_ENTITY_SPEED(PLAYER::PLAYER_PED_ID()) < 0.2f &&			//Not moving
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
	if (Settings::gamepadDirectInput)
		gamepad.InitDirectInput(GetForegroundWindow());

	//Get reference
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	//Change PlayerPed
	ENTITY::SET_ENTITY_COLLISION(playerPed, FALSE, FALSE);

	RADAR::SET_BLIP_SPRITE(RADAR::GET_MAIN_PLAYER_BLIP_ID(), 4109568128, TRUE);
	RADAR::SET_BLIP_NAME_FROM_TEXT_FILE(RADAR::GET_MAIN_PLAYER_BLIP_ID(), "BLIP_PLAYER");
	
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

	RADAR::SET_BLIP_SPRITE(RADAR::GET_MAIN_PLAYER_BLIP_ID(), 3536528039, TRUE);
	RADAR::SET_BLIP_NAME_FROM_TEXT_FILE(RADAR::GET_MAIN_PLAYER_BLIP_ID(), "BLIP_PLAYER");

	//Additional
	RADAR::UNLOCK_MINIMAP_ANGLE();
	ENTITY::FREEZE_ENTITY_POSITION(playerPed, FALSE);
	PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), FALSE);

	delete drone;
	drone = nullptr;

	gamepad.ReleaseDirectInput();

	//Wait for loading world
	ULONGLONG waitTime = 500 + ULONGLONG(distance);
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

	RADAR::SET_BLIP_SPRITE(RADAR::GET_MAIN_PLAYER_BLIP_ID(), 3536528039, TRUE);
	RADAR::SET_BLIP_NAME_FROM_TEXT_FILE(RADAR::GET_MAIN_PLAYER_BLIP_ID(), "BLIP_PLAYER");

	//Additional
	RADAR::UNLOCK_MINIMAP_ANGLE();
	ENTITY::FREEZE_ENTITY_POSITION(playerPed, FALSE);
	PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), FALSE);

	delete drone;
	drone = nullptr;

	gamepad.ReleaseDirectInput();

	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, FALSE, FALSE, 0);
}

void disableFlightButtons()
{
	CONTROLS::DISABLE_CONTROL_ACTION(0, GAMEPLAY_X::JOAAT("INPUT_OPEN_WHEEL_MENU"), TRUE);
}

void WAIT_LONG(ULONGLONG waitTime)
{
	static ULONGLONG maxTickCount;
	maxTickCount = GetTickCount64() + waitTime;
	do {
		WAIT(0);
		update();
	} while (GetTickCount64() < maxTickCount);
}

