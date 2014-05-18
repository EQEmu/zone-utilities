#ifndef EQEMU_COMMON_S3D_TEXTURE_H
#define EQEMU_COMMON_S3D_TEXTURE_H

#include <vector>
#include <string>
#include <memory>

namespace EQEmu
{

namespace S3D
{

class Texture
{
public:
	Texture() { }
	~Texture() { }
	
	std::vector<std::string> &GetTextureFrames() { return frames; }
private:
	std::vector<std::string> frames;
};

}

}

#endif
