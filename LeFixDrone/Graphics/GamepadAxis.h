#pragma once

#include "..\Input\Gamepad.h"

class GamepadAxis
{
public:
	GamepadAxis(Gamepad* _pad);
	~GamepadAxis();

	void draw();

private:
	Gamepad* pad;
	enum data { eR, eP, eY, eT, eB };
};