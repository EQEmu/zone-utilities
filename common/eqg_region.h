#ifndef EQEMU_COMMON_EQG_REGION_H
#define EQEMU_COMMON_EQG_REGION_H

namespace EQEmu
{

namespace EQG
{

class Region
{
public:
	Region() { x_scale = y_scale = z_scale = 1.0f; }
	~Region() { }

	void SetLocation(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	void SetRotation(float nx, float ny, float nz) { x_rot = nx; y_rot = ny; z_rot = nz; }
	void SetScale(float nx, float ny, float nz) { x_scale = nx; y_scale = ny; z_scale = nz; }
	void SetExtents(float nx, float ny, float nz) { x_ext = nx; y_ext = ny; z_ext = nz; }
	void SetName(std::string name) { this->name = name; }
	void SetFlags(uint32_t f1, uint32_t f2) { flag[0] = f1; flag[1] = f2; }

	float GetX() { return x; }
	float GetY() { return y; }
	float GetZ() { return z; }
	float GetRotationX() { return x_rot; }
	float GetRotationY() { return y_rot; }
	float GetRotationZ() { return z_rot; }
	float GetScaleX() { return x_scale; }
	float GetScaleY() { return y_scale; }
	float GetScaleZ() { return z_scale; }
	float GetExtentX() { return x_ext; }
	float GetExtentY() { return y_ext; }
	float GetExtentZ() { return z_ext; }
	uint32_t GetFlag1() { return flag[0]; }
	uint32_t GetFlag2() { return flag[0]; }
	std::string &GetName() { return name; }
private:
	float x, y, z;
	float x_ext, y_ext, z_ext;
	float x_rot, y_rot, z_rot;
	float x_scale, y_scale, z_scale;
	uint32_t flag[2];
	std::string name;
};

}

}

#endif
