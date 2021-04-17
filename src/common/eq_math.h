#ifndef EQEMU_COMMON_EQ_MATH_H
#define EQEMU_COMMON_EQ_MATH_H

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

glm::mat4 CreateRotateMatrix(float rx, float ry, float rz);
glm::mat4 CreateTranslateMatrix(float tx, float ty, float tz);
glm::mat4 CreateScaleMatrix(float sx, float sy, float sz);

float Distance(const glm::vec3& a, const glm::vec3& b);
float DistanceNoRoot(const glm::vec3& a, const glm::vec3& b);
float DistanceNoZ(const glm::vec3& a, const glm::vec3& b);
float DistanceNoRootNoZ(const glm::vec3& a, const glm::vec3& b);

#endif
