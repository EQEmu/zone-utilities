#ifndef EQEMU_COMMON_TEXTURE_BRUSH_H
#define EQEMU_COMMON_TEXTURE_BRUSH_H

#include "texture.h"

namespace EQEmu
{

class TextureBrush
{
public:
	TextureBrush() { flags = 0; }
	~TextureBrush() { }

	void SetFlags(uint32_t f) { flags = f; }

	std::vector<std::shared_ptr<Texture>> &GetTextures() { return brush_textures; }
	uint32_t GetFlags() { return flags; }
private:
	std::vector<std::shared_ptr<Texture>> brush_textures;
	uint32_t flags;
};

}

#endif
