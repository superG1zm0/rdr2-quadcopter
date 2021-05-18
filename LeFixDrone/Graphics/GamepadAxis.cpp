#include "GamepadAxis.h"



GamepadAxis::GamepadAxis(Gamepad* _pad) : pad(_pad)
{
}

GamepadAxis::~GamepadAxis()
{
}

void GamepadAxis::draw()
{
	static ColorRGBA colAxis[5] = { ColorRGBA(255, 0, 0, 200), ColorRGBA(0, 255, 0, 200), ColorRGBA(255, 0, 255, 200), ColorRGBA(255, 255, 0, 200), ColorRGBA(255, 165, 0, 200) };
	static ColorRGBA colRct = ColorRGBA(0, 0, 0, 80);
	static ColorRGBA colBrd = ColorRGBA(0, 0, 0, 120);
	static ColorRGBA colLine = ColorRGBA(0, 0, 0, 255);
	static ColorRGBA colDeadBand = ColorRGBA(255, 255, 255, 80);

	static float widthRct = 0.6f;  //Width of Graph Retangle
	static float heightRct = 0.5f;  //Height of Graph Retangle
	static float sizeBrd = 0.01f; //Border Thickness

	static float posX = 0.5f; //X Position of Center
	static float posY = 0.4f; //Y Position of Center

	static float text_scale = 0.8f * widthRct; //Size

	static float sizeLine = 0.005f;//Size of Lines
	static int nbrLines = 11;//Number of Lines

	float ratio = GRAPHICS_X::GET_ASPECT_RATIO();

	//Draw background
	GRAPHICS_X::DRAW_RECT(posX, posY - 0.5f * (heightRct + sizeBrd), (widthRct + 2 * sizeBrd) / ratio, sizeBrd, colBrd); //Top
	GRAPHICS_X::DRAW_RECT(posX, posY + 0.5f * (heightRct + sizeBrd), (widthRct + 2 * sizeBrd) / ratio, sizeBrd, colBrd); //Bottom
	GRAPHICS_X::DRAW_RECT(posX - 0.5f * (widthRct + sizeBrd) / ratio, posY, sizeBrd / ratio, heightRct, colBrd); //Left
	GRAPHICS_X::DRAW_RECT(posX + 0.5f * (widthRct + sizeBrd) / ratio, posY, sizeBrd / ratio, heightRct, colBrd); //Right

	GRAPHICS_X::DRAW_RECT(posX, posY, widthRct / ratio, heightRct, colRct);


	float textPosX = posX + (sizeBrd - 0.5f * widthRct) / ratio;
	float lineOffsetX = 0.08f / ratio;
	float lengthLine = (widthRct - sizeBrd * 4.0f) / ratio - lineOffsetX * 2.0f;

	UI_X::DRAW_TEXT("ROLL", textPosX, posY + (0.5f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 255, 0, 0, 200, false, text_scale);
	UI_X::DRAW_TEXT("PITCH", textPosX, posY + (2.5f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 0, 255, 0, 200, false, text_scale);
	UI_X::DRAW_TEXT("YAW", textPosX, posY + (4.5f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 255, 0, 255, 200, false, text_scale);
	UI_X::DRAW_TEXT("THROTTLE", textPosX, posY + (6.5f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 255, 255, 0, 200, false, text_scale);
	UI_X::DRAW_TEXT("UNSTUCK", textPosX, posY + (8.0f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 255, 165, 0, 200, false, text_scale);
	UI_X::DRAW_TEXT("FLIP", textPosX, posY + (9.0f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 255, 165, 0, 200, false, text_scale);
	UI_X::DRAW_TEXT("CAM", textPosX, posY + (10.0f - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines - text_scale * 0.04f, 255, 165, 0, 200, false, text_scale);

	//Draw lines
	for (int i = 0; i < (nbrLines - 3); i++)
		GRAPHICS_X::DRAW_RECT(posX + lineOffsetX, posY + (i - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine, sizeLine, colLine);

	float raw_roll = (pad->raw_stick[0] + 1.0f) / 2.0f;
	float raw_pitch = (pad->raw_stick[1] + 1.0f) / 2.0f;
	float raw_yaw = (pad->raw_stick[2] + 1.0f) / 2.0f;
	float raw_throttle = (pad->raw_stick[3] + 1.0f) / 2.0f;

	float roll = (pad->stick_right_x + 1.0f) / 2.0f;
	float pitch = (pad->stick_right_y + 1.0f) / 2.0f;
	float yaw = (pad->stick_left_x + 1.0f) / 2.0f;
	float throttle = (pad->stick_left_y + 1.0f) / 2.0f;

	//Draw raw axis
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (raw_roll - 1.0f) / 2.0f, posY + (0 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * raw_roll, sizeLine, colAxis[eR]);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (raw_pitch - 1.0f) / 2.0f, posY + (2 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * raw_pitch, sizeLine, colAxis[eP]);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (raw_yaw - 1.0f) / 2.0f, posY + (4 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * raw_yaw, sizeLine, colAxis[eY]);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (raw_throttle - 1.0f) / 2.0f, posY + (6 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * raw_throttle, sizeLine, colAxis[eT]);

	//Draw axis
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (roll - 1.0f) / 2.0f, posY + (1 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * roll, sizeLine, colAxis[eR]);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (pitch - 1.0f) / 2.0f, posY + (3 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * pitch, sizeLine, colAxis[eP]);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (yaw - 1.0f) / 2.0f, posY + (5 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * yaw, sizeLine, colAxis[eY]);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * (throttle - 1.0f) / 2.0f, posY + (7 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * throttle, sizeLine, colAxis[eT]);


	float Deadband[] = { Settings::gamepadRollDeadband / 1000.0f,
		Settings::gamepadPitchDeadband / 1000.0f,
		Settings::gamepadYawDeadband / 1000.0f
	};

	float ThrottleCenter = Settings::gamepadThrottleCenter / 1000.0f;
	float Center[] = { Settings::gamepadRollCenter / 1000.0f,
		Settings::gamepadPitchCenter / 1000.0f,
		Settings::gamepadYawCenter / 1000.0f };

	for (int i = 0; i < 3; i++)
	{
		float tmp_center;
		float tmp_width;
		float tmp_min = Center[i] - Deadband[i];
		float tmp_max = Center[i] + Deadband[i];

		if (tmp_min < -1.0f)
		{
			tmp_center = Center[i] - (tmp_min + 1.0f) / 2.0f;
			tmp_width = Deadband[i] + (tmp_min + 1.0f) / 2.0f;
		}
		else if (tmp_max > 1.0f)
		{
			tmp_center = Center[i] - (tmp_max - 1.0f) / 2.0f;
			tmp_width = Deadband[i] - (tmp_max - 1.0f) / 2.0f;
		}
		else
		{
			tmp_center = Center[i];
			tmp_width = Deadband[i];
		}
		GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * tmp_center / 2.0f, posY + (i * 2 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine * tmp_width, sizeLine * 6.0f, colDeadBand);
		GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * Center[i] / 2.0f, posY + (i * 2 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, sizeLine / ratio, sizeLine * 6.0f, colLine);
		GRAPHICS_X::DRAW_RECT(posX + lineOffsetX, posY + (i * 2 + 1 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, sizeLine / ratio, sizeLine * 6.0f, colLine);
	}

	//THROTTLE
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX + lengthLine * ThrottleCenter / 2.0f, posY + (6 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, sizeLine / ratio, sizeLine * 6.0f, colLine);
	GRAPHICS_X::DRAW_RECT(posX + lineOffsetX, posY + (7 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, sizeLine / ratio, sizeLine * 6.0f, colLine);

	//BUTTONS
	for (int i = 0; i < 3; i++)
	{
		if(pad->raw_button[i])
			GRAPHICS_X::DRAW_RECT(posX + lineOffsetX, posY + (i + 8 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine, sizeLine, colAxis[eB]);
		else
			GRAPHICS_X::DRAW_RECT(posX + lineOffsetX, posY + (i + 8 - nbrLines / 2.0f + 0.5f) * heightRct / nbrLines, lengthLine, sizeLine, colLine);
	}
}