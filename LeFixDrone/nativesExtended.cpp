
#include "nativesExtended.h"

std::string toString(const Eigen::Vector3f &v)
{
	return "X:" + std::to_string(v.x()) + " Y:" + std::to_string(v.y()) + " Z:" + std::to_string(v.z());
}
std::string toString(const Eigen::Quaternionf &q)
{
	return toString(q.vec()) + " W:" + std::to_string(q.w());
}

Eigen::Vector3f Convert(const Vector3 &v){
	return Eigen::Vector3f(v.x, v.y, v.z);
}

float getHeading(const Eigen::Vector3f &v)
{
	return 180.0f + RadToDeg( atan2f(v.x(), -v.y()) );
};

float RadToDeg(const float &rad)
{
	return float(rad / M_PI * 180.0);
}

float DegToRad(const float &deg)
{
	return float(deg * M_PI / 180.0);
}

//ENTITY

Eigen::Quaternionf ENTITY_X::GET_ENTITY_QUATERNION(Entity e)
{
	// NOT WORK
	//float var[4];
	//ENTITY::GET_ENTITY_QUATERNION(e, var+0, var+1, var+2, var+3);
	return Eigen::Quaternionf(1.0f, 0.0f, 0.0f, 0.0f);
}

void ENTITY_X::SET_ENTITY_QUATERNION(Entity e, Eigen::Quaternionf q)
{
	//Set rotation at center
	Eigen::Vector3f pos, right, old_up, new_up, forward;

	ENTITY_X::GET_ENTITY_MATRIX(e, &right, &forward, &old_up, &pos);
	ENTITY::SET_ENTITY_QUATERNION(e, q.x(), q.y(), q.z(), q.w());
	ENTITY_X::GET_ENTITY_MATRIX(e, &right, &forward, &new_up, &pos);

	Eigen::Vector3f up = old_up - new_up;

	Eigen::Vector3f dim = GAMEPLAY_X::GET_MODEL_DIMENSIONS(e);

	Eigen::Vector3f new_pos = pos + up * dim.z() / 2.0f;

	ENTITY::SET_ENTITY_COORDS(e, new_pos.x(), new_pos.y(), new_pos.z(), FALSE, FALSE, FALSE, FALSE);
}

Eigen::Vector3f ENTITY_X::GET_ENTITY_COORDS(Entity e)
{
	//Get coords at center
	Eigen::Vector3f upVector = ENTITY_X::GET_ENTITY_UP_VECTOR(e);
	Eigen::Vector3f dim = GAMEPLAY_X::GET_MODEL_DIMENSIONS(e);
	Eigen::Vector3f pos = upVector * dim.z() / 2.0f;

	return Convert(ENTITY::GET_ENTITY_COORDS(e, true, false)) + pos;
}

void ENTITY_X::GET_ENTITY_MATRIX(Entity e, Eigen::Vector3f* rightVector, Eigen::Vector3f* forwardVector, Eigen::Vector3f* upVector, Eigen::Vector3f* pos)
{
	Vector3 tmp_rightVector, temp_forwardVector, temp_upVector, temp_pos;

	ENTITY::GET_ENTITY_MATRIX(e, &tmp_rightVector, &temp_forwardVector, &temp_upVector, &temp_pos);

	*rightVector = Convert(tmp_rightVector);
	*forwardVector = Convert(temp_forwardVector);
	*upVector = Convert(temp_upVector);
	*pos = Convert(temp_pos);
}

Eigen::Vector3f ENTITY_X::GET_ENTITY_UP_VECTOR(Entity e)
{
	Vector3 tmp_rightVector, temp_forwardVector, temp_upVector, temp_pos;

	ENTITY::GET_ENTITY_MATRIX(e, &tmp_rightVector, &temp_forwardVector, &temp_upVector, &temp_pos);

	return Convert(temp_upVector);
}

Eigen::Vector3f ENTITY_X::GET_ENTITY_VELOCITY(Entity e) 
{
	return Convert(ENTITY::GET_ENTITY_VELOCITY(e, false));
}

void ENTITY_X::SET_ENTITY_COORDS(Entity e, Eigen::Vector3f pos)
{
	//Set coords at center
	Eigen::Vector3f upVector = ENTITY_X::GET_ENTITY_UP_VECTOR(e);
	Eigen::Vector3f dim = GAMEPLAY_X::GET_MODEL_DIMENSIONS(e);
	Eigen::Vector3f deltaPos = upVector * dim.z() / 2.0f;

	ENTITY::SET_ENTITY_COORDS(e, pos.x() - deltaPos.x(), pos.y() - deltaPos.y(), pos.z() - deltaPos.z(), FALSE, FALSE, FALSE, FALSE);
}

void ENTITY_X::SET_ENTITY_VELOCITY(Entity e, Eigen::Vector3f vel)
{
	ENTITY::SET_ENTITY_VELOCITY(e, vel.x(), vel.y(), vel.z());
}

void ENTITY_X::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(Entity entity, Eigen::Vector3f force, bool isDirRel, bool isStrong, bool isMassRel)
{
	int forceFlags = 0u;
	if (isStrong) forceFlags |= (1u << 0); //Set first bit

										   //LAST BOOL HAS TO BE FALSE (SCRIPT STOPS RUNNING)
	ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(entity, forceFlags, force.x(), force.y(), force.z(), FALSE, isDirRel, isMassRel, FALSE);
}

void ENTITY_X::APPLY_FORCE_TO_ENTITY(Entity entity, Eigen::Vector3f force, Eigen::Vector3f offset, int boneIndex, bool isDirRel, bool isStrong, bool isMassRel)
{
	int forceFlags = 0u;
	if (isStrong) forceFlags |= (1u << 0); //Set first bit

	ENTITY::APPLY_FORCE_TO_ENTITY(entity, forceFlags, force.x(), force.y(), force.z(), offset.x(), offset.y(), offset.z(), boneIndex, isDirRel, TRUE, isMassRel, FALSE, TRUE);
}

void ENTITY_X::APPLY_MOMENTUM_TO_ENTITY(Entity entity, Eigen::Vector3f momentum, int boneIndex, bool isStrong, bool isMassRel)
{
	//This method has no isDirRel setting!
	//For the workaround I would have to pass or read the objects rotation to calculate the global force pair.
	//And I don't need global momentums for my mod...

	float momSqr = momentum.squaredNorm();

	//Why is a zero momentum vector stopping body rotation?
	//Following code produces zero vectors (force and offset) which shouldn't stop object roation.
	if (momSqr < 0.0001f*0.0001f) return; //Workaround: Don't apply forces at all when momentum is near zero.

	Eigen::Vector3f offset = momentum.cross(Eigen::Vector3f(1.0f, 0.0f, 0.0f));

	if (offset.squaredNorm() < sqrtf(0.5f)*momSqr) {
		offset = momentum.cross(Eigen::Vector3f(0.0f, 1.0f, 0.0f)); //Crossproduct with (0/1/0) has to better than (1/0/0)
	}
	offset.normalize();


	// SECOND: calculate the force vector(s)
	Eigen::Vector3f force = momentum.cross(offset);


	// THIRD: force flags, momentums behave strange can't properly use them yet
	int forceFlags = 0u;
	if (isStrong) forceFlags |= (1u << 0); //Set first bit
	//forceFlags |= (1u << 2); //Set third bit

	// FOURTH: Apply Forces which are physically the same as the desired momentum
	ENTITY_X::APPLY_FORCE_TO_ENTITY(entity, force*(+1.0f), offset*(+0.5f), boneIndex, true, isStrong, isMassRel);
	ENTITY_X::APPLY_FORCE_TO_ENTITY(entity, force*(-1.0f), offset*(-0.5f), boneIndex, true, isStrong, isMassRel);
	//ENTITY_X::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(entity, forceFlags, momentum.x(), momentum.y(), momentum.z(), FALSE, isDirRel, isMassRel, FALSE); //y u no work
}

//CAM

void CAM_X::SET_CAM_QUATERNION(Camera c, Eigen::Quaternionf q)
{
	//Eigen::Matrix3f rot = q.toRotationMatrix();

	//float ax = asinf(rot(1,0));
	//float ay = atan2f(rot(2,0), rot(2,1));
	//float az = atan2f(rot(0,0), rot(0,1));

	float r11 = -2 * (q.x() * q.y() - q.w() * q.z());
	float r12 = q.w() * q.w() - q.x() * q.x() + q.y() * q.y() - q.z() * q.z();
	float r21 = 2 * (q.y() * q.z() + q.w() * q.x());
	float r31 = -2 * (q.x() * q.z() - q.w() * q.y());
	float r32 = q.w() * q.w() - q.x() * q.x() - q.y() * q.y() + q.z() * q.z();
	
	float ax = asinf(r21);
	float ay = atan2f(r31, r32);
	float az = atan2f(r11, r12);

	const float f = 360.0f / 2.0f / 3.1415926535897f;
	ax *= f;
	ay *= f;
	az *= f;

	CAM::SET_CAM_ROT(c, ax, ay, az, 2);
}

Eigen::Quaternionf CAM_X::GET_CAM_QUATERNION(Camera c)
{
	Eigen::Vector3f rotVec = DegToRad(1.0f)*Convert(CAM::GET_CAM_ROT(c, 0));

	Eigen::Matrix3f xRot = Eigen::AngleAxisf(rotVec.x(), Eigen::Vector3f(1.0f, 0.0f, 0.0f)).matrix();
	Eigen::Matrix3f yRot = Eigen::AngleAxisf(rotVec.y(), Eigen::Vector3f(0.0f, 1.0f, 0.0f)).matrix();
	Eigen::Matrix3f zRot = Eigen::AngleAxisf(rotVec.z(), Eigen::Vector3f(0.0f, 0.0f, 1.0f)).matrix();

	Eigen::Matrix3f rot = zRot*yRot*xRot;

	return Eigen::Quaternionf(rot);
}

void CAM_X::SET_CAM_COORD(Camera c, Eigen::Vector3f pos)
{
	CAM::SET_CAM_COORD(c, pos.x(), pos.y(), pos.z());
}

Eigen::Vector3f CAM_X::GET_CAM_COORD(Camera c)
{
	return Eigen::Vector3f(Convert(CAM::GET_CAM_COORD(c)));
}

void CAM_X::ATTACH_CAM_TO_ENTITY(Camera c, Entity e, Eigen::Vector3f pos, bool isRel)
{
	CAM::ATTACH_CAM_TO_ENTITY(c, e, pos.x(), pos.y(), pos.z(), isRel ? TRUE : FALSE);
}

void CAM_X::POINT_CAM_AT_COORD(Camera c, Eigen::Vector3f pos)
{
	CAM::POINT_CAM_AT_COORD(c, pos.x(), pos.y(), pos.z());
}

//GAMEPLAY

Hash GAMEPLAY_X::JOAAT(char* string)
{
	return GAMEPLAY::GET_HASH_KEY(string);
}

Eigen::Vector3f GAMEPLAY_X::GET_MODEL_DIMENSIONS(Entity e)
{
	Vector3 min, max;

	GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(e), &min, &max);
	return Convert(max) - Convert(min);
}

//UILOG

void UILOG_X::PRINT_SUBTITLE(const char* text)
{
	char* literalString = GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", (char*)text);
	UILOG::_0xFA233F8FE190514C((Any*)literalString);
	UILOG::_0xE9990552DEC71600();
	UILOG::_0xDFF0D417277B41F8();
}

//GRAPHICS

void GRAPHICS_X::DRAW_RECT(float posX, float posY, float width, float height, ColorRGBA col)
{
	GRAPHICS::DRAW_RECT(posX, posY, width, height, col.r, col.g, col.b, col.a, FALSE, FALSE);
}

void GRAPHICS_X::DRAW_RECT(float x, float y, float width, float height, int r, int g, int b, int a)
{
	float fX = x + width / 2;
	float fY = y + height / 2;
	GRAPHICS::DRAW_RECT(fX, fY, width, height, r, g, b, a, FALSE, FALSE);
}

void GRAPHICS_X::DRAW_DOT(float posX, float posY, float size, ColorRGBA col)
{
	float ratio = GRAPHICS_X::GET_ASPECT_RATIO();
	DRAW_RECT(posX, posY, size / ratio, size, col);
}

void GRAPHICS_X::DRAW_SPRITE(const char* category, const char* sprite, float x, float y, float scalex, float scaley, float rotation, int r, int g, int b, int a)
{
	float fX = x + scalex / 2;
	float fY = y + scaley / 2;
	if (!TEXTURE::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)sprite))
		TEXTURE::REQUEST_STREAMED_TEXTURE_DICT((char*)sprite, 0);
	GRAPHICS::DRAW_SPRITE((char*)category, (char*)sprite, fX, fY, scalex, scaley, rotation, r, g, b, a, 1);
	TEXTURE::SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED((char*)category);
}

float GRAPHICS_X::GET_ASPECT_RATIO(void)
{
	int x, y;

	GRAPHICS::GET_SCREEN_RESOLUTION(&x, &y);

	return (float)x / (float)y;
}

//UI

void UI_X::DRAW_TEXT(const char* text, float x, float y, int r, int g, int b, int a, bool centered, float size)
{
	UI::SET_TEXT_COLOR_RGBA(r, g, b, a);
	UI::SET_TEXT_SCALE(0.0f, size);
	UI::SET_TEXT_CENTRE(centered);
	char* literalString = GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", (char*)text);
	UI::DRAW_TEXT(literalString, x, y);
}

void UI_X::SET_TEXT_COLOUR(ColorRGBA col)
{
	UI::SET_TEXT_COLOR_RGBA(col.r, col.g, col.b, col.a);
}

void UI_X::SET_TEXT_DROPSHADOW(int distance, ColorRGBA col)
{
	UI::SET_TEXT_DROPSHADOW(distance, col.r, col.g, col.b, col.a);
}