#ifndef EQEMU_COMMON_PLACEABLE_H
#define EQEMU_COMMON_PLACEABLE_H

namespace EQEmu
{

class Placeable
{
public:
	Placeable() { }
	~Placeable() { }

	void SetLocation(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	void SetRotation(float nx, float ny, float nz) { rotate_x = nx; rotate_y = ny; rotate_z = nz; }
	void SetScale(float nx, float ny, float nz) { scale_x = nx; scale_y = ny; scale_z = nz; }
	void SetName(std::string name) { model_name = name; }
	void SetFileName(std::string name) { model_file = name; }

	float GetX() { return x; }
	float GetY() { return y; }
	float GetZ() { return z; }
	float GetRotateX() { return rotate_x; }
	float GetRotateY() { return rotate_y; }
	float GetRotateZ() { return rotate_z; }
	float GetScaleX() { return scale_x; }
	float GetScaleY() { return scale_y; }
	float GetScaleZ() { return scale_z; }
	std::string &GetName() { return model_name; }
	std::string &GetFileName() { return model_file; }
private:
	float x, y, z;
	float rotate_x, rotate_y, rotate_z;
	float scale_x, scale_y, scale_z;
	std::string model_name;
	std::string model_file;
};

}

#endif
