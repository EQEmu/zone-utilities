#ifndef EQEMU_COMMON_S3D_TEXTURE_BRUSH_H
#define EQEMU_COMMON_S3D_TEXTURE_BRUSH_H

#include "s3d_texture.h"

namespace EQEmu
{

namespace S3D
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

}

#endif
