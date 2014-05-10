#include "overlay.h"
#include <FreeImage.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

TextureAtlas::TextureAtlas() {
	texture_id = 0;
	texture_data = nullptr;
}

TextureAtlas::~TextureAtlas() {
	if(texture_id != 0)
		glDeleteTextures(1, &texture_id);

	if (texture_data)
		delete[] texture_data;
}

bool TextureAtlas::Init(std::string atlas_file, std::string atlas_config) {
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(atlas_file.c_str());
	FIBITMAP *bitmap = FreeImage_Load(format, atlas_file.c_str());
	if(!bitmap) {
		return false;
	}

	FIBITMAP *converted_bitmap = FreeImage_ConvertTo32Bits(bitmap);
	FreeImage_Unload(bitmap);

	if(!converted_bitmap) {
		return false;
	}

	int width = FreeImage_GetWidth(converted_bitmap);
	int height = FreeImage_GetHeight(converted_bitmap);

	if(width <= 0 || height <= 0) {
		FreeImage_Unload(converted_bitmap);
		return false;
	}

	texture_data = new GLubyte[4 * width * height];
	char* pixel_data = (char*)FreeImage_GetBits(converted_bitmap);
	for (int j = 0; j < width * height; j++) {
		texture_data[j * 4 + 0] = pixel_data[j * 4 + 2];
		texture_data[j * 4 + 1] = pixel_data[j * 4 + 1];
		texture_data[j * 4 + 2] = pixel_data[j * 4 + 0];
		texture_data[j * 4 + 3] = pixel_data[j * 4 + 3];
	}

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)texture_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	FreeImage_Unload(converted_bitmap);
	
	//load config data here...
	
	return true;
}

OverlayManager::OverlayManager() {
	dirty = false;
	vbo[0] = 0;
	vbo[1] = 0;
	vbo[2] = 0;
	vao = 0;
	ib = 0;
}

OverlayManager::~OverlayManager() {
	if (vbo[0]) {
		glDeleteBuffers(1, &vbo[0]);
	}

	if (vbo[1]) {
		glDeleteBuffers(1, &vbo[1]);
	}

	if (vbo[2]) {
		glDeleteBuffers(1, &vbo[2]);
	}

	if (ib) {
		glDeleteBuffers(1, &ib);
	}

	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}
}

bool OverlayManager::Init(std::string atlas_file, std::string atlas_config) {
	atlas.reset(new TextureAtlas());

	if(!atlas->Init(atlas_file, atlas_config)) {
		atlas.reset();
		return false;
	}

#ifndef EQEMU_GL_DEP
	overlay_shader = ShaderProgram("shaders/overlay.vert", "shaders/overlay.frag");
#else
	overlay_shader = ShaderProgram("shaders/overlay130.vert", "shaders/overlay130.frag");
#endif
	overlay_mvp = overlay_shader.GetUniformLocation("mvp");
	overlay_tex_sampler = overlay_shader.GetUniformLocation("tex_sample");
	return true;
}

void OverlayManager::Draw(uint32_t screen_width, uint32_t screen_height) {
	if(dirty) {
		Compile();
	}

	if (vertex_buffer_pos.size() == 0 || !atlas) {
		return;
	}
	
	glDisable(GL_DEPTH_TEST);

	overlay_shader.Use();
	glm::mat4 overlay_proj_mat = glm::ortho(0.0f, (float)screen_width, float(screen_height), 0.0f);
	overlay_mvp.SetValueMatrix4(1, false, &overlay_proj_mat[0][0]);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas->GetTextureID());
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	overlay_tex_sampler.SetValue((int)0);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)index_buffer.size(), GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void OverlayManager::Compile() {
	dirty = false;
	
	vertex_buffer_pos.clear();
	vertex_buffer_uv.clear();
	vertex_buffer_color.clear();
	index_buffer.clear();

	size_t sz = overlays.size();
	for(size_t i = 0; i < sz; ++i) {
		auto overlay = overlays[i];
		overlay->AddGeometry(vertex_buffer_pos, vertex_buffer_uv, vertex_buffer_color, index_buffer);
	}

	if(vertex_buffer_pos.size() == 0)
		return;

	if (vbo[0]) {
		glDeleteBuffers(1, &vbo[0]);
		vbo[0] = 0;
	}

	if (vbo[1]) {
		glDeleteBuffers(1, &vbo[1]);
		vbo[1] = 0;
	}

	if (vbo[2]) {
		glDeleteBuffers(1, &vbo[2]);
		vbo[2] = 0;
	}

	if (ib) {
		glDeleteBuffers(1, &ib);
		ib = 0;
	}

	if (vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_pos.size() * sizeof(glm::vec3), &vertex_buffer_pos[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vbo[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_uv.size() * sizeof(glm::vec2), &vertex_buffer_uv[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vbo[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_color.size() * sizeof(glm::vec3), &vertex_buffer_color[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer.size() * sizeof(uint16_t), &index_buffer[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBindVertexArray(0);
}
