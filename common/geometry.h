#ifndef EQEMU_GEOMETRY_H
#define EQEMU_GEOMETRY_H

#include "glm.hpp"
#include <vector>
#include "texture_brush_set.h"

class Geometry
{
public:
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 tex;
		glm::vec3 nor;
	};

	struct Polygon
	{
		uint32_t flags;
		uint32_t verts[3];
		uint32_t tex;
	};

	Geometry() { }
	~Geometry() { }

	void AddVertex(Vertex v) { verts.push_back(v); }
	void AddPolygon(Polygon p) { polys.push_back(p); }
	void SetName(std::string nname) { name = nname; }
	void SetTextureBrushSet(std::shared_ptr<TextureBrushSet> tbs) { tex = tbs; }

	std::vector<Vertex> &GetVertices() { return verts; }
	std::vector<Polygon> &GetPolygons() { return polys; }
	std::string &GetName() { return name; }
	std::shared_ptr<TextureBrushSet> GetTextureBrushSet() { return tex; }
private:
	std::vector<Vertex> verts;
	std::vector<Polygon> polys;
	std::string name;
	std::shared_ptr<TextureBrushSet> tex;
};

#endif
