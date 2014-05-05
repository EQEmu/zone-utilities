#ifndef EQEMU_TEXTURE_BRUSH_SET_H
#define EQEMU_TEXTURE_BRUSH_SET_H

#include "texture_brush.h"

class TextureBrushSet
{
public:
	TextureBrushSet() { }
	~TextureBrushSet() { }

	void AddTextureSet(std::shared_ptr<TextureBrush> tex) { texture_sets.push_back(tex); }

	std::vector<std::shared_ptr<TextureBrush>> &GetTextureSet() { return texture_sets; }
private:
	std::vector<std::shared_ptr<TextureBrush>> texture_sets;
};

#endif
