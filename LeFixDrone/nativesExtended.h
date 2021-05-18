#pragma once

#define _USE_MATH_DEFINES

#include "inc\types.h"
#include "inc\natives.h"
#include "inc\enums.h"

#include <Eigen/Dense>
#include <string>
#include <cmath>

#include "Graphics\ColorRGBA.h"

typedef int Prompt;

std::string toString(const Eigen::Vector3f &v);
std::string toString(const Eigen::Quaternionf &q);

Eigen::Vector3f Convert(const Vector3 &v);

float getHeading(const Eigen::Vector3f &v);

float RadToDeg(const float &rad);
float DegToRad(const float &deg);

namespace ENTITY_X
{
	Eigen::Quaternionf GET_ENTITY_QUATERNION(Entity e);
	void GET_ENTITY_MATRIX(Entity e, Eigen::Vector3f* rightVector, Eigen::Vector3f* forwardVector, Eigen::Vector3f* upVector, Eigen::Vector3f* pos);
	Eigen::Vector3f GET_ENTITY_UP_VECTOR(Entity e);
	void SET_ENTITY_QUATERNION(Entity e, Eigen::Quaternionf q);
	Eigen::Vector3f GET_ENTITY_COORDS(Entity e);
	Eigen::Vector3f GET_ENTITY_VELOCITY(Entity e);
	void SET_ENTITY_COORDS(Entity e, Eigen::Vector3f);
	void SET_ENTITY_VELOCITY(Entity e, Eigen::Vector3f vel);
	void APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(Entity entity, Eigen::Vector3f force, bool isDirRel, bool isStrong, bool isMassRel);
	void APPLY_FORCE_TO_ENTITY(Entity entity, Eigen::Vector3f force, Eigen::Vector3f offset, int boneIndex, bool isDirRel, bool isStrong, bool isMassRel);
	void APPLY_MOMENTUM_TO_ENTITY(Entity entity, Eigen::Vector3f momentum, int boneIndex, bool isStrong, bool isMassRel);
}

namespace CAM_X
{
	void POINT_CAM_AT_COORD(Camera c, Eigen::Vector3f pos);
	void SET_CAM_QUATERNION(Camera c, Eigen::Quaternionf q);
	Eigen::Quaternionf GET_CAM_QUATERNION(Camera c);
	void SET_CAM_COORD(Camera c, Eigen::Vector3f pos);
	Eigen::Vector3f GET_CAM_COORD(Camera c);
	void ATTACH_CAM_TO_ENTITY(Camera c, Entity e, Eigen::Vector3f pos, bool isRel);
}

namespace GAMEPLAY_X
{
	Hash JOAAT(char* string);
	Eigen::Vector3f GET_MODEL_DIMENSIONS(Entity e);
}

namespace UILOG_X
{
	void PRINT_SUBTITLE(const char* text);
}

namespace GRAPHICS_X
{
	void DRAW_RECT(float posX, float posY, float width, float height, ColorRGBA col);
	void DRAW_RECT(float x, float y, float width, float height, int r, int g, int b, int a);
	void DRAW_DOT(float posX, float posY, float size, ColorRGBA col);
	void DRAW_SPRITE(const char* category, const char* sprite, float x, float y, float scalex, float scaley, float rotation, int r, int g, int b, int a);
	float GET_ASPECT_RATIO(void);
}

namespace UI_X
{
	void DRAW_TEXT(const char* text, float x, float y, int r, int g, int b, int a, bool centered = false, float size = 0.342f);
	void SET_TEXT_COLOUR(ColorRGBA col);
	void SET_TEXT_DROPSHADOW(int distance, ColorRGBA col);

	Blip _BLIP_ADD_FOR_ENTITY(Hash blipHash, Entity entity);

	Prompt CREATE_PROMPT(const char* text, Hash control);
	void _UIPROMPT_SET_ENABLED(Prompt prompt, BOOL toggle);
	void _UIPROMPT_SET_VISIBLE(Prompt prompt, BOOL toggle);
	BOOL _UIPROMPT_IS_ENABLED(Prompt prompt);
}
