#ifndef EQEMU_COMMON_S3D_TEXTURE_BRUSH_SET_H
#define EQEMU_COMMON_S3D_TEXTURE_BRUSH_SET_H

#include "s3d_texture_brush.h"

namespace EQEmu
{

namespace S3D
{

class TextureBrushSet
{
public:
	TextureBrushSet() { }
	~TextureBrushSet() { }

	std::vector<std::shared_ptr<TextureBrush>> &GetTextureSet() { return texture_sets; }
private:
	std::vector<std::shared_ptr<TextureBrush>> texture_sets;
};

}

}

#endif
