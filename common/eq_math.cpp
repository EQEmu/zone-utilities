#include "eq_math.h"

glm::mat4 CreateRotateMatrix(float rx, float ry, float rz) {
	glm::mat4 rot_x(1.0f);
	rot_x[1][1] = cos(rx);
	rot_x[2][1] = -sin(rx);
	rot_x[1][2] = sin(rx);
	rot_x[2][2] = cos(rx);

	glm::mat4 rot_y(1.0f);
	rot_y[0][0] = cos(ry);
	rot_y[2][0] = sin(ry);
	rot_y[0][2] = -sin(ry);
	rot_y[2][2] = cos(ry);

	glm::mat4 rot_z(1.0f);
	rot_z[0][0] = cos(rz);
	rot_z[1][0] = -sin(rz);
	rot_z[0][1] = sin(rz);
	rot_z[1][1] = cos(rz);

	return rot_z * rot_y * rot_x;
}

glm::mat4 CreateTranslateMatrix(float tx, float ty, float tz) {
	glm::mat4 trans(1.0f);
	trans[3][0] = tx;
	trans[3][1] = ty;
	trans[3][2] = tz;

	return trans;
}

glm::mat4 CreateScaleMatrix(float sx, float sy, float sz) {
	glm::mat4 scale(1.0f);
	scale[0][0] = sx;
	scale[1][1] = sy;
	scale[2][2] = sz;
	return scale;
}

float Distance(const glm::vec3 &a, const glm::vec3 &b)
{
	float xdiff = a.x - b.x;
	float ydiff = a.y - b.y;
	float zdiff = a.z - b.z;
	return ::sqrt((xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff));
 }

float DistanceNoRoot(const glm::vec3 &a, const glm::vec3 &b)
{
	float xdiff = a.x - b.x;
	float ydiff = a.y - b.y;
	float zdiff = a.z - b.z;
	return (xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff);
}

float DistanceNoZ(const glm::vec3 &a, const glm::vec3 &b)
{
	float xdiff = a.x - b.x;
	float ydiff = a.y - b.y;
	return ::sqrt((xdiff * xdiff) + (ydiff * ydiff));
}

float DistanceNoRootNoZ(const glm::vec3 &a, const glm::vec3 &b)
{
	float xdiff = a.x - b.x;
	float ydiff = a.y - b.y;
	return (xdiff * xdiff) + (ydiff * ydiff);
}