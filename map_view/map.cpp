#include "map.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <tuple>
#include <map>
#include "compression.h"

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
			verts.push_back(glm::vec3(QuadVertex1Y, QuadVertex1X, QuadVertex1Z));
			verts.push_back(glm::vec3(QuadVertex2Y, QuadVertex2X, QuadVertex2Z));
			verts.push_back(glm::vec3(QuadVertex3Y, QuadVertex3X, QuadVertex3Z));
			verts.push_back(glm::vec3(QuadVertex4Y, QuadVertex4X, QuadVertex4Z));
			
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

				//uint32_t current_vert = (uint32_t)verts.size() + 3;
				uint32_t i1, i2, i3, i4;
#ifdef INVERSEXY
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
#else
				std::tuple<float, float, float> t = std::make_tuple(QuadVertex1Y, QuadVertex1X, QuadVertex1Z);
				auto iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i1 = iter->second;
				}
				else {
					i1 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex1Y, QuadVertex1X, QuadVertex1Z))
						cur_verts[t] = i1;
				}

				t = std::make_tuple(QuadVertex2Y, QuadVertex2X, QuadVertex2Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i2 = iter->second;
				}
				else {
					i2 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex2Y, QuadVertex2X, QuadVertex2Z));
					cur_verts[t] = i2;
				}

				t = std::make_tuple(QuadVertex3Y, QuadVertex3X, QuadVertex3Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i3 = iter->second;
				}
				else {
					i3 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex3Y, QuadVertex3X, QuadVertex3Z));
					cur_verts[t] = i3;
				}

				t = std::make_tuple(QuadVertex4Y, QuadVertex4X, QuadVertex4Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i4 = iter->second;
				}
				else {
					i4 = (uint32_t)verts.size();
					verts.push_back(glm::vec3(QuadVertex4Y, QuadVertex4X, QuadVertex4Z));
					cur_verts[t] = i4;
				}

				verts.push_back(glm::vec3(QuadVertex1Y, QuadVertex1X, QuadVertex1Z));
				verts.push_back(glm::vec3(QuadVertex2Y, QuadVertex2X, QuadVertex2Z));
				verts.push_back(glm::vec3(QuadVertex3Y, QuadVertex3X, QuadVertex3Z));
				verts.push_back(glm::vec3(QuadVertex4Y, QuadVertex4X, QuadVertex4Z));
#endif
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
	FILE *f = fopen(filename.c_str(), "rb");
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