
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
	float var[4];
	ENTITY::GET_ENTITY_QUATERNION(e, var+0, var+1, var+2, var+3);
	return Eigen::Quaternionf(var[3], var[0], var[1], var[2]);
}

void ENTITY_X::SET_ENTITY_QUATERNION(Entity e, Eigen::Quaternionf q)
{
	ENTITY::SET_ENTITY_QUATERNION(e, q.x(), q.y(), q.z(), q.w());
}

Eigen::Vector3f ENTITY_X::GET_ENTITY_COORDS(Entity e)
{
	return Convert(ENTITY::GET_ENTITY_COORDS(e, true));
}

Eigen::Vector3f ENTITY_X::GET_ENTITY_VELOCITY(Entity e) 
{
	return Convert(ENTITY::GET_ENTITY_VELOCITY(e));
}

void ENTITY_X::SET_ENTITY_COORDS(Entity e, Eigen::Vector3f pos)
{
	ENTITY::SET_ENTITY_COORDS(e, pos.x(), pos.y(), pos.z(), FALSE, FALSE, FALSE, FALSE);
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

void GRAPHICS_X::DRAW_RECT(float posX, float posY, float width, float height, ColorRGBA col)
{
	GRAPHICS::DRAW_RECT(posX, posY, width, height, col.r, col.g, col.b, col.a, FALSE);
}

void GRAPHICS_X::DRAW_DOT(float posX, float posY, float size, ColorRGBA col)
{
	float ratio = GRAPHICS::_GET_ASPECT_RATIO(FALSE);
	DRAW_RECT(posX, posY, size / ratio, size, col);
}

void UI_X::DRAW_TEXT(float posX, float posY, float scale, int font, ColorRGBA col, std::string text)
{
	HUD::SET_TEXT_FONT(font);
	HUD::SET_TEXT_SCALE(0.0, scale);
	UI_X::SET_TEXT_COLOUR(col);
	HUD::SET_TEXT_CENTRE(FALSE);
	UI_X::SET_TEXT_DROPSHADOW(0, ColorRGBA::transparent());
	UI_X::SET_TEXT_EDGE(0, ColorRGBA::transparent());
	HUD::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((LPSTR)text.c_str());
	HUD::END_TEXT_COMMAND_DISPLAY_TEXT(posX, posY, 0);
}

void UI_X::SET_TEXT_COLOUR(ColorRGBA col)
{
	HUD::SET_TEXT_COLOUR(col.r, col.g, col.b, col.a);
}

void UI_X::SET_TEXT_DROPSHADOW(int distance, ColorRGBA col)
{
	HUD::SET_TEXT_DROPSHADOW(distance, col.r, col.g, col.b, col.a);
}

void UI_X::SET_TEXT_EDGE(Hash p0, ColorRGBA col)
{
	HUD::SET_TEXT_EDGE(p0, col.r, col.g, col.g, col.a);
}