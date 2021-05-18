#pragma once
#include <Windows.h>

#include <vector>
#include <string>

class Settings {
public:
	static void SetFile(const std::string &filepath);
	static void Save();
	static void Load();

	//AUDIO
	static float audioVolume;

	//VISUAL
	static bool showCollider, showModel, showTrails, showStickCam, hideHUD;

	//GAMEPAD
	static bool gamepadVib, gamepadDirectInput;
	static int gamepadRollAxis, gamepadPitchAxis, gamepadYawAxis, gamepadThrottleAxis;
	static bool gamepadInvRoll, gamepadInvPitch, gamepadInvYaw, gamepadInvThrottle;
	static int gamepadRollDeadband, gamepadPitchDeadband, gamepadYawDeadband;
	static int gamepadRollCenter, gamepadPitchCenter, gamepadYawCenter, gamepadThrottleCenter;
	static float gamepadRollSensitivity, gamepadPitchSensitivity, gamepadYawSensitivity, gamepadThrottleSensitivity;
	static int gamepadUnstuckButton, gamepadFlipButton, gamepadCamButton;

	//CAMERA
	static int camMode;
	static int camDrone1FOV, camDrone1Tilt, camDrone3FOV, camDrone3Tilt, camClone1CloseFOV, camClone1FarFOV;
	static float camDrone3YPos, camDrone3ZPos;

	//CONTROL
	static float contRcRatePR, contRcRateY;
	static float contRateP, contRateR, contRateY;
	static float contExpoPR, contExpoY;

	//PID
	static bool pidEnable;
	static float pidP, pidI, pidD;

	//DRONE
	static float droneMass, droneMaxRelLoad, droneMaxVel;
	static bool drone3DFly, droneAcroMode, droneNoThrustDown;

	//PHYSICS
	static float physxGScale;
	static bool physxColl;

private:
	static std::string _filepath;
};
