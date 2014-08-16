#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include <stdio.h>
#include <stdint.h>
#include <string>

#define BEST_Z_INVALID -99999

class Map
{
public:
#pragma pack(1)
	struct Vertex
	{
		Vertex() : x(0.0f), y(0.0f), z(0.0f) { }
		Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }
		~Vertex() { }
		bool operator==(const Vertex &v) const 
		{
			return((v.x == x) && (v.y == y) && (v.z == z));
		}

		float x;
		float y;
		float z;
	};
#pragma pack()

	Map();
	~Map();
	
	float FindBestZ(Vertex &start, Vertex *result) const;
	bool LineIntersectsZone(Vertex start, Vertex end, float step, Vertex *result) const;
	bool LineIntersectsZoneNoZLeaps(Vertex start, Vertex end, float step_mag, Vertex *result) const;
	bool CheckLoS(Vertex myloc, Vertex oloc) const;
	bool Load(std::string filename);
	static Map *LoadMapFile(std::string file);
private:
	void RotateVertex(Vertex &v, float rx, float ry, float rz);
	void ScaleVertex(Vertex &v, float sx, float sy, float sz);
	void TranslateVertex(Vertex &v, float tx, float ty, float tz);
	bool LoadV1(FILE *f);
	bool LoadV2(FILE *f);
	
	struct impl;
	impl *imp;
};

#endif
