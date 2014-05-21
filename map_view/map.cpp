#include "map.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <tuple>
#include <map>
#include <memory>
#include "compression.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>

struct ModelEntry
{
	struct Poly
	{
		uint32_t v1, v2, v3;
		uint8_t vis;
	};
	std::vector<glm::vec3> verts;
	std::vector<Poly> polys;
};

bool LoadMapV1(FILE *f, std::vector<glm::vec3> &verts, std::vector<uint32_t> &indices) {
	uint32_t face_count;
	uint16_t node_count;
	uint32_t facelist_count;

	if (fread(&face_count, sizeof(face_count), 1, f) != 1) {
		return false;
	}

	if (fread(&node_count, sizeof(node_count), 1, f) != 1) {
		return false;
	}

	if (fread(&facelist_count, sizeof(facelist_count), 1, f) != 1) {
		return false;
	}

	verts.reserve(face_count * 3);
	indices.reserve(face_count * 3);
	std::map<std::tuple<float, float, float>, uint32_t> cur_verts;
	for (uint32_t i = 0; i < face_count; ++i) {
		glm::vec3 a;
		glm::vec3 b;
		glm::vec3 c;
		float normals[4];
		if (fread(&a, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if (fread(&b, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if (fread(&c, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if (fread(normals, sizeof(normals), 1, f) != 1) {
			return false;
		}

		std::tuple<float, float, float> t = std::make_tuple(a.x, a.y, a.z);
		auto iter = cur_verts.find(t);
		if (iter != cur_verts.end()) {
			indices.push_back(iter->second);
		}
		else {
			size_t sz = verts.size();
			verts.push_back(a);
			indices.push_back((uint32_t)sz);
			cur_verts[t] = (uint32_t)sz;
		}

		t = std::make_tuple(b.x, b.y, b.z);
		iter = cur_verts.find(t);
		if (iter != cur_verts.end()) {
			indices.push_back(iter->second);
		}
		else {
			size_t sz = verts.size();
			verts.push_back(b);
			indices.push_back((uint32_t)sz);
			cur_verts[t] = (uint32_t)sz;
		}

		t = std::make_tuple(c.x, c.y, c.z);
		iter = cur_verts.find(t);
		if (iter != cur_verts.end()) {
			indices.push_back(iter->second);
		}
		else {
			size_t sz = verts.size();
			verts.push_back(c);
			indices.push_back((uint32_t)sz);
			cur_verts[t] = (uint32_t)sz;
		}
	}

	for (uint16_t i = 0; i < node_count; ++i) {
		float min_x;
		float min_y;
		float max_x;
		float max_y;
		uint8_t flags;
		uint16_t nodes[4];

		if (fread(&min_x, sizeof(min_x), 1, f) != 1) {
			return false;
		}

		if (fread(&min_y, sizeof(min_y), 1, f) != 1) {
			return false;
		}

		if (fread(&max_x, sizeof(max_x), 1, f) != 1) {
			return false;
		}

		if (fread(&max_y, sizeof(max_y), 1, f) != 1) {
			return false;
		}

		if (fread(&flags, sizeof(flags), 1, f) != 1) {
			return false;
		}

		if (fread(nodes, sizeof(nodes), 1, f) != 1) {
			return false;
		}
	}

	for (uint32_t i = 0; i < facelist_count; ++i) {
		uint32_t facelist;

		if (fread(&facelist, sizeof(facelist), 1, f) != 1) {
			return false;
		}
	}

	return true;
}

void RotateVertex(glm::vec3 &v, float rx, float ry, float rz) {
	glm::vec3 nv = v;

	nv.y = (cos(rx) * v.y) - (sin(rx) * v.z);
	nv.z = (sin(rx) * v.y) + (cos(rx) * v.z);

	v = nv;

	nv.x = (cos(ry) * v.x) + (sin(ry) * v.z);
	nv.z = -(sin(ry) * v.x) + (cos(ry) * v.z);

	v = nv;

	nv.x = (cos(rz) * v.x) - (sin(rz) * v.y);
	nv.y = (sin(rz) * v.x) + (cos(rz) * v.y);

	v = nv;
}

void ScaleVertex(glm::vec3 &v, float sx, float sy, float sz) {
	v.x = v.x * sx;
	v.y = v.y * sy;
	v.z = v.z * sz;
}

void TranslateVertex(glm::vec3 &v, float tx, float ty, float tz) {
	v.x = v.x + tx;
	v.y = v.y + ty;
	v.z = v.z + tz;
}

glm::mat4 CreateRotateMatrix(float rx, float ry, float rz) {
	glm::mat4 rot_x(1.0f);
	rot_x[1][1] = cos(rx);
	rot_x[2][1] = -sin(rx);
	rot_x[1][2] = sin(rx);
	rot_x[2][2] = cos(rx);

	glm::mat4 rot_y(1.0f);
	rot_y[0][0] = cos(ry);
	rot_y[2][0] = sin(ry);
	rot_y[0][2] = -sin(ry);
	rot_y[2][2] = cos(ry);

	glm::mat4 rot_z(1.0f);
	rot_z[0][0] = cos(rz);
	rot_z[1][0] = -sin(rz);
	rot_z[0][1] = sin(rz);
	rot_z[1][1] = cos(rz);

	return rot_z * rot_y * rot_x;
}

glm::mat4 CreateTranslateMatrix(float tx, float ty, float tz) {
	glm::mat4 trans(1.0f);
	trans[3][0] = tx;
	trans[3][1] = ty;
	trans[3][2] = tz;

	return trans;
}

glm::mat4 CreateScaleMatrix(float sx, float sy, float sz) {
	glm::mat4 scale(1.0f);
	scale[0][0] = sx;
	scale[1][1] = sy;
	scale[2][2] = sz;
	return scale;
}

bool LoadMapV2(FILE *f, std::vector<glm::vec3> &verts, std::vector<uint32_t> &indices, std::vector<glm::vec3> &nc_verts, std::vector<uint32_t> &nc_indices) {
	verts.clear();
	indices.clear();
	nc_verts.clear();
	nc_indices.clear();

	uint32_t data_size;
	if (fread(&data_size, sizeof(data_size), 1, f) != 1) {
		return false;
	}

	uint32_t buffer_size;
	if (fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
		return false;
	}

	std::vector<char> data;
	data.resize(data_size);
	if (fread(&data[0], data_size, 1, f) != 1) {
		return false;
	}

	std::vector<char> buffer;
	buffer.resize(buffer_size);
	uint32_t v = EQEmu::InflateData(&data[0], data_size, &buffer[0], buffer_size);

	char *buf = &buffer[0];
	uint32_t vert_count;
	uint32_t ind_count;
	uint32_t nc_vert_count;
	uint32_t nc_ind_count;
	uint32_t model_count;
	uint32_t plac_count;
	uint32_t plac_group_count;
	uint32_t tile_count;
	uint32_t quads_per_tile;
	float units_per_vertex;
	
	vert_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	ind_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	nc_vert_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	nc_ind_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	model_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	plac_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	plac_group_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	tile_count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	quads_per_tile = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	units_per_vertex = *(float*)buf;
	buf += sizeof(float);

	for(uint32_t i = 0; i < vert_count; ++i) {
		float x;
		float y;
		float z;

		x = *(float*)buf;
		buf += sizeof(float);

		y = *(float*)buf;
		buf += sizeof(float);

		z = *(float*)buf;
		buf += sizeof(float);
	
		glm::vec3 vert(x, y, z);
		verts.push_back(vert);
	}
	
	for (uint32_t i = 0; i < ind_count; ++i) {
		uint32_t index;
		index = *(uint32_t*)buf;
		buf += sizeof(uint32_t);
	
		indices.push_back(index);
	}
	
	for (uint32_t i = 0; i < nc_vert_count; ++i) {
		float x;
		float y;
		float z;
		x = *(float*)buf;
		buf += sizeof(float);

		y = *(float*)buf;
		buf += sizeof(float);

		z = *(float*)buf;
		buf += sizeof(float);
	
		glm::vec3 vert(x, y, z);
		nc_verts.push_back(vert);
	}
	
	for (uint32_t i = 0; i < nc_ind_count; ++i) {
		uint32_t index;
		index = *(uint32_t*)buf;
		buf += sizeof(uint32_t);
	
		nc_indices.push_back(index);
	}

	std::map<std::string, std::shared_ptr<ModelEntry>> models;
	for (uint32_t i = 0; i < model_count; ++i) {
		std::shared_ptr<ModelEntry> me(new ModelEntry);
		std::string name = buf;
		buf += name.length() + 1;

		uint32_t vert_count = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		uint32_t poly_count = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		me->verts.resize(vert_count);
		for(uint32_t j = 0; j < vert_count; ++j) {
			float x = *(float*)buf;
			buf += sizeof(float);
			float y = *(float*)buf;
			buf += sizeof(float);
			float z = *(float*)buf;
			buf += sizeof(float);

			me->verts[j] = glm::vec3(x, y, z);
		}

		me->polys.resize(poly_count);
		for (uint32_t j = 0; j < poly_count; ++j) {
			uint32_t v1 = *(uint32_t*)buf;
			buf += sizeof(uint32_t);
			uint32_t v2 = *(uint32_t*)buf;
			buf += sizeof(uint32_t);
			uint32_t v3 = *(uint32_t*)buf;
			buf += sizeof(uint32_t);
			uint8_t vis = *(uint8_t*)buf;
			buf += sizeof(uint8_t);

			ModelEntry::Poly p;
			p.v1 = v1;
			p.v2 = v2;
			p.v3 = v3;
			p.vis = vis;
			me->polys[j] = p;
		}

		models[name] = me;
	}

	for (uint32_t i = 0; i < plac_count; ++i) {
		std::string name = buf;
		buf += name.length() + 1;

		float x = *(float*)buf;
		buf += sizeof(float);
		float y = *(float*)buf;
		buf += sizeof(float);
		float z = *(float*)buf;
		buf += sizeof(float);

		float x_rot = *(float*)buf;
		buf += sizeof(float);
		float y_rot = *(float*)buf;
		buf += sizeof(float);
		float z_rot = *(float*)buf;
		buf += sizeof(float);

		float x_scale = *(float*)buf;
		buf += sizeof(float);
		float y_scale = *(float*)buf;
		buf += sizeof(float);
		float z_scale = *(float*)buf;
		buf += sizeof(float);

		if(models.count(name) == 0)
			continue;

		auto model = models[name];
		auto &mod_polys = model->polys;
		auto &mod_verts = model->verts;
		for (uint32_t j = 0; j < mod_polys.size(); ++j) {
			auto &current_poly = mod_polys[j];
			auto v1 = mod_verts[current_poly.v1];
			auto v2 = mod_verts[current_poly.v2];
			auto v3 = mod_verts[current_poly.v3];

			RotateVertex(v1, x_rot, y_rot, z_rot);
			RotateVertex(v2, x_rot, y_rot, z_rot);
			RotateVertex(v3, x_rot, y_rot, z_rot);

			ScaleVertex(v1, x_scale, y_scale, z_scale);
			ScaleVertex(v2, x_scale, y_scale, z_scale);
			ScaleVertex(v3, x_scale, y_scale, z_scale);

			TranslateVertex(v1, x, y, z);
			TranslateVertex(v2, x, y, z);
			TranslateVertex(v3, x, y, z);

			float t = v1.x;
			v1.x = v1.y;
			v1.y = t;

			t = v2.x;
			v2.x = v2.y;
			v2.y = t;

			t = v3.x;
			v3.x = v3.y;
			v3.y = t;

			if (current_poly.vis == 0) {
				nc_verts.push_back(v1);
				nc_verts.push_back(v2);
				nc_verts.push_back(v3);

				nc_indices.push_back((uint32_t)nc_verts.size() - 3);
				nc_indices.push_back((uint32_t)nc_verts.size() - 2);
				nc_indices.push_back((uint32_t)nc_verts.size() - 1);
			}
			else {
				verts.push_back(v1);
				verts.push_back(v2);
				verts.push_back(v3);

				indices.push_back((uint32_t)verts.size() - 3);
				indices.push_back((uint32_t)verts.size() - 2);
				indices.push_back((uint32_t)verts.size() - 1);
			}
		}
	}

	for (uint32_t i = 0; i < plac_group_count; ++i) {
		float x = *(float*)buf;
		buf += sizeof(float);
		float y = *(float*)buf;
		buf += sizeof(float);
		float z = *(float*)buf;
		buf += sizeof(float);

		float x_rot = *(float*)buf;
		buf += sizeof(float);
		float y_rot = *(float*)buf;
		buf += sizeof(float);
		float z_rot = *(float*)buf;
		buf += sizeof(float);

		float x_scale = *(float*)buf;
		buf += sizeof(float);
		float y_scale = *(float*)buf;
		buf += sizeof(float);
		float z_scale = *(float*)buf;
		buf += sizeof(float);

		float x_tile = *(float*)buf;
		buf += sizeof(float);
		float y_tile = *(float*)buf;
		buf += sizeof(float);
		float z_tile = *(float*)buf;
		buf += sizeof(float);

		uint32_t p_count = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		for (uint32_t j = 0; j < p_count; ++j) {
			std::string name = buf;
			buf += name.length() + 1;

			float p_x = *(float*)buf;
			buf += sizeof(float);
			float p_y = *(float*)buf;
			buf += sizeof(float);
			float p_z = *(float*)buf;
			buf += sizeof(float);

			float p_x_rot = *(float*)buf * 3.14159f / 180;
			buf += sizeof(float);
			float p_y_rot = *(float*)buf * 3.14159f / 180;
			buf += sizeof(float);
			float p_z_rot = *(float*)buf * 3.14159f / 180;
			buf += sizeof(float);

			float p_x_scale = *(float*)buf;
			buf += sizeof(float);
			float p_y_scale = *(float*)buf;
			buf += sizeof(float);
			float p_z_scale = *(float*)buf;
			buf += sizeof(float);

			if (models.count(name) == 0)
				continue;

			auto &model = models[name];

			for(size_t k = 0; k < model->polys.size(); ++k) {
				auto &poly = model->polys[k];
				glm::vec3 v1, v2, v3;

				v1 = model->verts[poly.v1];
				v2 = model->verts[poly.v2];
				v3 = model->verts[poly.v3];

				ScaleVertex(v1, p_x_scale, p_y_scale, p_z_scale);
				ScaleVertex(v2, p_x_scale, p_y_scale, p_z_scale);
				ScaleVertex(v3, p_x_scale, p_y_scale, p_z_scale);

				TranslateVertex(v1, p_x, p_y, p_z);
				TranslateVertex(v2, p_x, p_y, p_z);
				TranslateVertex(v3, p_x, p_y, p_z);

				RotateVertex(v1, x_rot * 3.14159f / 180.0f, 0, 0);
				RotateVertex(v2, x_rot * 3.14159f / 180.0f, 0, 0);
				RotateVertex(v3, x_rot * 3.14159f / 180.0f, 0, 0);

				RotateVertex(v1, 0, y_rot * 3.14159f / 180.0f, 0);
				RotateVertex(v2, 0, y_rot * 3.14159f / 180.0f, 0);
				RotateVertex(v3, 0, y_rot * 3.14159f / 180.0f, 0);

				glm::vec3 correction(p_x, p_y, p_z);

				RotateVertex(correction, x_rot * 3.14159f / 180.0f, 0, 0);

				TranslateVertex(v1, -correction.x, -correction.y, -correction.z);
				TranslateVertex(v2, -correction.x, -correction.y, -correction.z);
				TranslateVertex(v3, -correction.x, -correction.y, -correction.z);

				RotateVertex(v1, p_x_rot, 0, 0);
				RotateVertex(v2, p_x_rot, 0, 0);
				RotateVertex(v3, p_x_rot, 0, 0);

				RotateVertex(v1, 0, -p_y_rot, 0);
				RotateVertex(v2, 0, -p_y_rot, 0);
				RotateVertex(v3, 0, -p_y_rot, 0);

				RotateVertex(v1, 0, 0, p_z_rot);
				RotateVertex(v2, 0, 0, p_z_rot);
				RotateVertex(v3, 0, 0, p_z_rot);

				TranslateVertex(v1, correction.x, correction.y, correction.z);
				TranslateVertex(v2, correction.x, correction.y, correction.z);
				TranslateVertex(v3, correction.x, correction.y, correction.z);

				RotateVertex(v1, 0, 0, z_rot * 3.14159f / 180.0f);
				RotateVertex(v2, 0, 0, z_rot * 3.14159f / 180.0f);
				RotateVertex(v3, 0, 0, z_rot * 3.14159f / 180.0f);

				ScaleVertex(v1, x_scale, y_scale, z_scale);
				ScaleVertex(v2, x_scale, y_scale, z_scale);
				ScaleVertex(v3, x_scale, y_scale, z_scale);

				TranslateVertex(v1, x_tile, y_tile, z_tile);
				TranslateVertex(v2, x_tile, y_tile, z_tile);
				TranslateVertex(v3, x_tile, y_tile, z_tile);

				TranslateVertex(v1, x, y, z);
				TranslateVertex(v2, x, y, z);
				TranslateVertex(v3, x, y, z);

				float t = v1.x;
				v1.x = v1.y;
				v1.y = t;

				t = v2.x;
				v2.x = v2.y;
				v2.y = t;

				t = v3.x;
				v3.x = v3.y;
				v3.y = t;

				if (poly.vis == 0) {
					nc_verts.push_back(v1);
					nc_verts.push_back(v2);
					nc_verts.push_back(v3);

					nc_indices.push_back((uint32_t)nc_verts.size() - 3);
					nc_indices.push_back((uint32_t)nc_verts.size() - 2);
					nc_indices.push_back((uint32_t)nc_verts.size() - 1);
				}
				else {
					verts.push_back(v1);
					verts.push_back(v2);
					verts.push_back(v3);

					indices.push_back((uint32_t)verts.size() - 3);
					indices.push_back((uint32_t)verts.size() - 2);
					indices.push_back((uint32_t)verts.size() - 1);
				}
			}
		}
	}

	uint32_t ter_quad_count = (quads_per_tile * quads_per_tile);
	uint32_t ter_vert_count = ((quads_per_tile + 1) * (quads_per_tile + 1));
	std::vector<uint8_t> flags;
	std::vector<float> floats;
	flags.resize(ter_quad_count);
	floats.resize(ter_vert_count);
	for (uint32_t i = 0; i < tile_count; ++i) {
		bool flat;
		flat = *(bool*)buf;
		buf += sizeof(bool);

		float x;
		x = *(float*)buf;
		buf += sizeof(float);

		float y;
		y = *(float*)buf;
		buf += sizeof(float);

		if(flat) {
			float z;
			z = *(float*)buf;
			buf += sizeof(float);

			float QuadVertex1X = x;
			float QuadVertex1Y = y;
			float QuadVertex1Z = z;

			float QuadVertex2X = QuadVertex1X + (quads_per_tile * units_per_vertex);
			float QuadVertex2Y = QuadVertex1Y;
			float QuadVertex2Z = QuadVertex1Z;

			float QuadVertex3X = QuadVertex2X;
			float QuadVertex3Y = QuadVertex1Y + (quads_per_tile * units_per_vertex);
			float QuadVertex3Z = QuadVertex1Z;

			float QuadVertex4X = QuadVertex1X;
			float QuadVertex4Y = QuadVertex3Y;
			float QuadVertex4Z = QuadVertex1Z;

			uint32_t current_vert = (uint32_t)verts.size() + 3;
			verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
			verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
			verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
			verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));
			
			indices.push_back(current_vert);
			indices.push_back(current_vert - 2);
			indices.push_back(current_vert - 1);

			indices.push_back(current_vert);
			indices.push_back(current_vert - 3);
			indices.push_back(current_vert - 2);
		} else {
			//read flags
			for (uint32_t j = 0; j < ter_quad_count; ++j) {
				uint8_t f;
				f = *(uint8_t*)buf;
				buf += sizeof(uint8_t);

				flags[j] = f;
			}

			//read floats
			for (uint32_t j = 0; j < ter_vert_count; ++j) {
				float f;
				f = *(float*)buf;
				buf += sizeof(float);

				floats[j] = f;
			}

			int row_number = -1;
			std::map<std::tuple<float, float, float>, uint32_t> cur_verts;
			for (uint32_t quad = 0; quad < ter_quad_count; ++quad) {
				if ((quad % quads_per_tile) == 0) {
					++row_number;
				}

				if(flags[quad] & 0x01)
					continue;

				float QuadVertex1X = x + (row_number * units_per_vertex);
				float QuadVertex1Y = y + (quad % quads_per_tile) * units_per_vertex;
				float QuadVertex1Z = floats[quad + row_number];

				float QuadVertex2X = QuadVertex1X + units_per_vertex;
				float QuadVertex2Y = QuadVertex1Y;
				float QuadVertex2Z = floats[quad + row_number + quads_per_tile + 1];

				float QuadVertex3X = QuadVertex1X + units_per_vertex;
				float QuadVertex3Y = QuadVertex1Y + units_per_vertex;
				float QuadVertex3Z = floats[quad + row_number + quads_per_tile + 2];

				float QuadVertex4X = QuadVertex1X;
				float QuadVertex4Y = QuadVertex1Y + units_per_vertex;
				float QuadVertex4Z = floats[quad + row_number + 1];

				uint32_t i1, i2, i3, i4;
				std::tuple<float, float, float> t = std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z);
				auto iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i1 = iter->second;
				} else {
					i1 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
					cur_verts[t] = i1;
				}

				t = std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i2 = iter->second;
				}
				else {
					i2 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
					cur_verts[t] = i2;
				}

				t = std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i3 = iter->second;
				}
				else {
					i3 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
					cur_verts[t] = i3;
				}

				t = std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i4 = iter->second;
				}
				else {
					i4 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));
					cur_verts[t] = i4;
				}

				indices.push_back(i4);
				indices.push_back(i2);
				indices.push_back(i3);
				
				indices.push_back(i4);
				indices.push_back(i1);
				indices.push_back(i2);
			}
		}
	}

	return true;
}

void LoadMap(std::string filename, Model **collision, Model **vision) {
	std::string raw_filename = filename + ".map";
	FILE *f = fopen(raw_filename.c_str(), "rb");
	if (f) {
		uint32_t version;
		if (fread(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			*collision = nullptr;
			*vision = nullptr;
		}

		if (version == 0x01000000) {
			Model *new_model = new Model();
			bool v = LoadMapV1(f, new_model->GetPositions(), new_model->GetIndicies());
			fclose(f);

			if (v && new_model->GetPositions().size() > 0) {
				new_model->Compile();
				*collision = new_model;
				*vision = nullptr;
			} else {
				delete new_model;
				*collision = nullptr;
				*vision = nullptr;
			}
		}
		else if (version == 0x02000000) {
			Model *new_model = new Model();
			Model *nc_new_model = new Model();
			bool v = LoadMapV2(f, new_model->GetPositions(), new_model->GetIndicies(), nc_new_model->GetPositions(), nc_new_model->GetIndicies());
			fclose(f);

			if (v) {
				if (new_model->GetPositions().size() > 0) {
					new_model->Compile();
					*collision = new_model;
				} else {
					delete new_model;
					*collision = nullptr;
				}

				if (nc_new_model->GetPositions().size() > 0) {
					nc_new_model->Compile();
					*vision = nc_new_model;
				}
				else {
					delete nc_new_model;
					*vision = nullptr;
				}
			}
			else {
				delete new_model;
				delete nc_new_model;
				*collision = nullptr;
				*vision = nullptr;
			}
		}
		else {
			fclose(f);
			*collision = nullptr;
			*vision = nullptr;
		}
	} else {
		*collision = nullptr;
		*vision = nullptr;
	}
}

void LoadWaterMap(std::string filename, Model **volume) {
	*volume = nullptr;

	std::string raw_filename = filename + ".wtr";
	FILE *f = fopen(raw_filename.c_str(), "rb");
	if (f) {
		char magic[10];
		uint32_t version;
		if (fread(magic, 10, 1, f) != 1) {
			fclose(f);
			*volume = nullptr;
			return;
		}

		if (strncmp(magic, "EQEMUWATER", 10)) {
			fclose(f);
			*volume = nullptr;
			return;
		}

		if (fread(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			*volume = nullptr;
			return;
		}

		if(version != 2) {
			fclose(f);
			*volume = nullptr;
			return;
		}

		uint32_t region_count;
		if (fread(&region_count, sizeof(region_count), 1, f) != 1) {
			fclose(f);
			*volume = nullptr;
			return;
		}

		if(region_count == 0) {
			fclose(f);
			*volume = nullptr;
			return;
		}

		*volume = new Model();

		for (uint32_t i = 0; i < region_count; ++i) {
			uint32_t region_type;
			float x;
			float y;
			float z;
			float tile_x;
			float tile_y;
			float tile_z;
			float x_rot;
			float y_rot;
			float z_rot;
			float x_scale;
			float y_scale;
			float z_scale;
			float x_extent;
			float y_extent;
			float z_extent;

			if (fread(&region_type, sizeof(region_type), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&x, sizeof(x), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&y, sizeof(y), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&z, sizeof(z), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&tile_x, sizeof(tile_x), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&tile_y, sizeof(tile_y), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&tile_z, sizeof(tile_z), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&x_rot, sizeof(x_rot), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&y_rot, sizeof(y_rot), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&z_rot, sizeof(z_rot), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&x_scale, sizeof(x_scale), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&y_scale, sizeof(y_scale), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&z_scale, sizeof(z_scale), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&x_extent, sizeof(x_extent), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&y_extent, sizeof(y_extent), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			if (fread(&z_extent, sizeof(z_extent), 1, f) != 1) {
				fclose(f);
				delete *volume;
				*volume = nullptr;
				return;
			}

			x_extent = fabs(x_extent);
			y_extent = fabs(y_extent);
			z_extent = fabs(z_extent);
		
			glm::vec4 v1(-x_extent, y_extent, -z_extent, 1.0f);
			glm::vec4 v2(-x_extent, y_extent, z_extent, 1.0f);
			glm::vec4 v3(x_extent, y_extent, z_extent, 1.0f);
			glm::vec4 v4(x_extent, y_extent, -z_extent, 1.0f);
			glm::vec4 v5(-x_extent, -y_extent, -z_extent, 1.0f);
			glm::vec4 v6(-x_extent, -y_extent, z_extent, 1.0f);
			glm::vec4 v7(x_extent, -y_extent, z_extent, 1.0f);
			glm::vec4 v8(x_extent, -y_extent, -z_extent, 1.0f);

			glm::mat4 transformation = CreateRotateMatrix(x_rot * 3.14159f / 180.0f, y_rot * 3.14159f / 180.0f, z_rot * 3.14159f / 180.0f);
			transformation = CreateScaleMatrix(x_scale, y_scale, z_scale) * transformation;
			transformation = CreateTranslateMatrix(x, y, z) * transformation;
			transformation = CreateTranslateMatrix(tile_x, tile_y, tile_z) * transformation;
			
			v1 = transformation * v1;
			v2 = transformation * v2;
			v3 = transformation * v3;
			v4 = transformation * v4;
			v5 = transformation * v5;
			v6 = transformation * v6;
			v7 = transformation * v7;
			v8 = transformation * v8;

			uint32_t current_index = (uint32_t)(*volume)->GetPositions().size();
			(*volume)->GetPositions().push_back(glm::vec3(v1.y, v1.x, v1.z));
			(*volume)->GetPositions().push_back(glm::vec3(v2.y, v2.x, v2.z));
			(*volume)->GetPositions().push_back(glm::vec3(v3.y, v3.x, v3.z));
			(*volume)->GetPositions().push_back(glm::vec3(v4.y, v4.x, v4.z));
			(*volume)->GetPositions().push_back(glm::vec3(v5.y, v5.x, v5.z));
			(*volume)->GetPositions().push_back(glm::vec3(v6.y, v6.x, v6.z));
			(*volume)->GetPositions().push_back(glm::vec3(v7.y, v7.x, v7.z));
			(*volume)->GetPositions().push_back(glm::vec3(v8.y, v8.x, v8.z));

			//top
			(*volume)->GetIndicies().push_back(current_index + 0);
			(*volume)->GetIndicies().push_back(current_index + 1);
			(*volume)->GetIndicies().push_back(current_index + 2);
			(*volume)->GetIndicies().push_back(current_index + 2);
			(*volume)->GetIndicies().push_back(current_index + 3);
			(*volume)->GetIndicies().push_back(current_index + 0);

			//back
			(*volume)->GetIndicies().push_back(current_index + 1);
			(*volume)->GetIndicies().push_back(current_index + 2);
			(*volume)->GetIndicies().push_back(current_index + 6);
			(*volume)->GetIndicies().push_back(current_index + 6);
			(*volume)->GetIndicies().push_back(current_index + 5);
			(*volume)->GetIndicies().push_back(current_index + 1);
			
			//bottom
			(*volume)->GetIndicies().push_back(current_index + 4);
			(*volume)->GetIndicies().push_back(current_index + 5);
			(*volume)->GetIndicies().push_back(current_index + 6);
			(*volume)->GetIndicies().push_back(current_index + 6);
			(*volume)->GetIndicies().push_back(current_index + 7);
			(*volume)->GetIndicies().push_back(current_index + 4);
			
			//front
			(*volume)->GetIndicies().push_back(current_index + 0);
			(*volume)->GetIndicies().push_back(current_index + 3);
			(*volume)->GetIndicies().push_back(current_index + 7);
			(*volume)->GetIndicies().push_back(current_index + 7);
			(*volume)->GetIndicies().push_back(current_index + 4);
			(*volume)->GetIndicies().push_back(current_index + 0);
			
			//left
			(*volume)->GetIndicies().push_back(current_index + 0);
			(*volume)->GetIndicies().push_back(current_index + 1);
			(*volume)->GetIndicies().push_back(current_index + 5);
			(*volume)->GetIndicies().push_back(current_index + 5);
			(*volume)->GetIndicies().push_back(current_index + 4);
			(*volume)->GetIndicies().push_back(current_index + 0);
			
			//right
			(*volume)->GetIndicies().push_back(current_index + 3);
			(*volume)->GetIndicies().push_back(current_index + 2);
			(*volume)->GetIndicies().push_back(current_index + 6);
			(*volume)->GetIndicies().push_back(current_index + 6);
			(*volume)->GetIndicies().push_back(current_index + 7);
			(*volume)->GetIndicies().push_back(current_index + 3);
		}

		(*volume)->Compile();
		fclose(f);
	}
}
