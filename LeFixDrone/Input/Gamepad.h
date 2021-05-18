#pragma once

#include "inc\natives.h"
#include "..\nativesExtended.h"
#include "inc\enums.h"
#include "..\Settings.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Gamepad
{

public:
	Gamepad();
	~Gamepad() {}

	bool button_unstuck;
	bool button_flip;
	bool button_cam;

	bool raw_button[3];

	float stick_left_x;
	float stick_left_y;
	float stick_right_x;
	float stick_right_y;

	float raw_stick[4];

	void refresh();

	HRESULT InitDirectInput(HWND hWnd);
	VOID    ReleaseDirectInput();
	CHAR    name[MAX_PATH];

private:
	static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,
		VOID* pContext);
	static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,
		VOID* pContext);
	HRESULT UpdateInputState();

	LPDIRECTINPUT8          g_pDI;
	LPDIRECTINPUTDEVICE8    g_pJoystick;
	DIJOYSTATE              js;
};
