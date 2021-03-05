#pragma once
#include "inc\types.h"
#include "inc\natives.h"

class TimeCycleManager
{
public:
	TimeCycleManager();
	~TimeCycleManager();

	static void setTimecycleFadeOut(char* modifierName, float _duration);
	static void setTimecycleFadeIn(char* modifierName, float _duration);
	static void update();

private:

	static bool isFadingOut;
	static bool isFadingIn;
	static float strength;
	static DWORD lastUpdate;
	static float duration;
};

