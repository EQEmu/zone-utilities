#ifndef EQEMU_OVERLAY_H
#define EQEMU_OVERLAY_H

#include <string>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include "shader.h"

class TextureAtlas 
{
public:
	TextureAtlas();
	~TextureAtlas();

	bool Init(std::string atlas_file, std::string atlas_config);

	GLuint GetTextureID() { return texture_id; }
private:
	GLuint texture_id;
	GLubyte* texture_data;
};

class OverlayManager;
class OverlayElement
{
public:
	OverlayElement() { x = y = 0.0f; manager = nullptr; }
	virtual ~OverlayElement() { }

	void SetLocation(float nx, float ny) { x = nx; y = ny; }
	float GetX() { return x; }
	float GetY() { return y; }

	void SetActiveOverlayManager(OverlayManager *m)  { manager = m; }

	virtual void AddGeometry(std::vector<float> &pos, std::vector<float> &uv, std::vector<float> &color, std::vector<uint16_t> &index) = 0;
private:
	float x, y;
	OverlayManager *manager;
};

class OverlayManager
{
public:
	OverlayManager();
	~OverlayManager();

	bool Init(std::string atlas_file, std::string atlas_config);

	void Draw(uint32_t screen_width, uint32_t screen_height);

	void AddElement(std::shared_ptr<OverlayElement> element) { overlays.push_back(element); element->SetActiveOverlayManager(this); ForceRedraw(); }

	void ForceRedraw() { dirty = true; }

	std::shared_ptr<TextureAtlas> GetTextureAtlas() { return atlas; }
private:
	void Compile();

	std::shared_ptr<TextureAtlas> atlas;
	std::vector<std::shared_ptr<OverlayElement>> overlays;
	std::vector<float> vertex_buffer_pos;
	std::vector<float> vertex_buffer_uv;
	std::vector<float> vertex_buffer_color;
	std::vector<uint16_t> index_buffer;
	bool dirty;

	ShaderProgram overlay_shader;
	ShaderUniform overlay_mvp;
	ShaderUniform overlay_tex_sampler;

	GLuint vao;
	GLuint vbo[3]; //pos, uv, color
	GLuint ib; //index_buffer
};

#endif
