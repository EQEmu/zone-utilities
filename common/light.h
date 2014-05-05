#ifndef EQEMU_LIGHT_H
#define EQEMU_LIGHT_H

class Light
{
public:
	Light() { }
	~Light() { }

	void SetLocation(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	void SetColor(float nr, float ng, float nb) { r = nr; g = ng; b = nb; }
	void SetRadius(float nrad) { rad = nrad; }

	float GetX() { return x; }
	float GetY() { return y; }
	float GetZ() { return z; }
	float GetR() { return r; }
	float GetG() { return g; }
	float GetB() { return b; }
	float GetRadius() { return rad; }
private:
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float rad;
};

#endif
