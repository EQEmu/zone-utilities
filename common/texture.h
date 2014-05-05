#ifndef EQEMU_TEXTURE_H
#define EQEMU_TEXTURE_H

#include <vector>
#include <string>
#include <memory>

class Texture
{
public:
	Texture() { }
	~Texture() { }

	void AddTextureFrame(std::string file_name) { frames.push_back(file_name); }

	std::vector<std::string> &GetTextureFrames();
private:
	std::vector<std::string> frames;
};

#endif
