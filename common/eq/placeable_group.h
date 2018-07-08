#ifndef EQEMU_COMMON_PLACEABLE_GROUP_H
#define EQEMU_COMMON_PLACEABLE_GROUP_H

#include "placeable.h"

namespace EQEmu
{

class PlaceableGroup
{
public:
	PlaceableGroup() { }
	~PlaceableGroup() { }

	void SetFromTOG(bool v) { from_tog = v; }
	void SetLocation(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	void SetTileLocation(float nx, float ny, float nz) { tile_x = nx; tile_y = ny; tile_z = nz; }
	void SetRotation(float nx, float ny, float nz) { rot_x = nx; rot_y = ny; rot_z = nz; }
	void SetScale(float nx, float ny, float nz) { scale_x = nx; scale_y = ny; scale_z = nz; }
	void AddPlaceable(std::shared_ptr<Placeable> p) { placeables.push_back(p); }

	bool GetFromTOG() { return from_tog; }
	float GetX() { return x; }
	float GetY() { return y; }
	float GetZ() { return z; }
	float GetTileX() { return tile_x; }
	float GetTileY() { return tile_y; }
	float GetTileZ() { return tile_z; }
	float GetRotationX() { return rot_x; }
	float GetRotationY() { return rot_y; }
	float GetRotationZ() { return rot_z; }
	float GetScaleX() { return scale_x; }
	float GetScaleY() { return scale_y; }
	float GetScaleZ() { return scale_z; }
	std::vector<std::shared_ptr<Placeable>>& GetPlaceables() { return placeables; }
private:
	bool from_tog;
	float x, y, z;
	float tile_x, tile_y, tile_z;
	float rot_x, rot_y, rot_z;
	float scale_x, scale_y, scale_z;
	std::vector<std::shared_ptr<Placeable>> placeables;
};

}

#endif
