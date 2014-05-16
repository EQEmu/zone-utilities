#ifndef EQEMU_COMMON_WATER_SHEET_H
#define EQEMU_COMMON_WATER_SHEET_H

namespace EQEmu
{

class WaterSheet
{
public:
	WaterSheet() { min_x = min_y = max_x = max_y = 0.0f; }
	~WaterSheet() { }

	void SetMinX(float x) { min_x = x; }
	void SetMinY(float y) { min_y = y; }
	void SetMaxX(float x) { max_x = x; }
	void SetMaxY(float y) { max_y = y; }
	void SetZHeight(float z) { z_height = z; }

	float GetMinX() { return min_x; }
	float GetMinY() { return min_y; }
	float GetMaxX() { return max_x; }
	float GetMaxY() { return max_y; }
	float GetZHeight() { return z_height; }
private:
	float min_x;
	float min_y;
	float max_x;
	float max_y;
	float z_height;
};

}

#endif
