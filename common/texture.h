#ifndef EQEMU_COMMON_TEXTURE_H
#define EQEMU_COMMON_TEXTURE_H

#include <vector>
#include <string>
#include <memory>

namespace EQEmu
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

#endif
