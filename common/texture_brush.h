#ifndef EQEMU_TEXTURE_BRUSH_H
#define EQEMU_TEXTURE_BRUSH_H

#include "texture.h"

class TextureBrush
{
public:
	TextureBrush() { flags = 0; }
	~TextureBrush() { }

	void AddTexture(std::shared_ptr<Texture> tex) { brush_textures.push_back(tex); }
	void SetFlags(uint32_t f) { flags = f; }

	std::vector<std::shared_ptr<Texture>> &GetTextures() { return brush_textures; }
	uint32_t GetFlags() { return flags; }
private:
	std::vector<std::shared_ptr<Texture>> brush_textures;
	uint32_t flags;
};

#endif
