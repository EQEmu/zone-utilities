#ifndef EQEMU_COMMON_INVIS_WALL_H
#define EQEMU_COMMON_INVIS_WALL_H

#include <string>
#include <vector>
#include <glm.hpp>

namespace EQEmu
{

class InvisWall
{
public:
	InvisWall() { }
	~InvisWall() { }

	void SetName(std::string n) { name = n; }

	std::string &GetName() { return name; }
	std::vector<glm::vec3> &GetVerts() { return verts; }
private:
	std::string name;
	std::vector<glm::vec3> verts;
};

}

#endif
