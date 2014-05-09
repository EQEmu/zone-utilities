#ifndef EQEMU_COMMON_TEXTURE_BRUSH_SET_H
#define EQEMU_COMMON_TEXTURE_BRUSH_SET_H

#include "texture_brush.h"

namespace EQEmu
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

#endif
