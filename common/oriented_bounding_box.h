#ifndef EQEMU_ORIENTED_BOUNDNG_BOX_H
#define EQEMU_ORIENTED_BOUNDNG_BOX_H

#include "eq_math.h"

class OrientedBoundingBox
{
public:
	OrientedBoundingBox() { }
	OrientedBoundingBox(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, glm::vec3 extents);
	~OrientedBoundingBox() { }

	bool ContainsPoint(glm::vec3 p) const;
	
	glm::mat4& GetTransformation() { return transformation; }
	glm::mat4& GetInvertedTransformation() { return inverted_transformation; }

	float GetMinX() const { return min_x; }
	float GetMinY() const { return min_y; }
	float GetMinZ() const { return min_z; }
	float GetMaxX() const { return max_x; }
	float GetMaxY() const { return max_y; }
	float GetMaxZ() const { return max_z; }
private:
	float min_x, max_x;
	float min_y, max_y;
	float min_z, max_z;
	glm::mat4 transformation;
	glm::mat4 inverted_transformation;
};

#endif
