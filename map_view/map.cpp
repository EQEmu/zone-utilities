#include "map.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <random>
#include <tuple>
#include <map>

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
			cur_verts[t] = sz;
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
			cur_verts[t] = sz;
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
			cur_verts[t] = sz;
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

void LoadMap(std::string filename, Model **collision, Model **liquid, Model **vision) {
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		uint32_t version;
		if (fread(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			*collision = nullptr;
			*liquid = nullptr;
			*vision = nullptr;
		}

		if (version == 0x01000000) {
			Model *new_model = new Model();
			bool v = LoadMapV1(f, new_model->GetPositions(), new_model->GetIndicies());
			fclose(f);

			if(v) {
				std::mt19937 gen;
				gen.seed(time(0));
				size_t color_count = new_model->GetPositions().size();
				for(size_t i = 0; i < color_count; ++i) {
					float color = 0.5f + (0.5f * ((float)gen() / (float)gen.max()));
					new_model->GetColors().push_back(glm::vec3(color, color, color));
				}

				new_model->Compile();
				*collision = new_model;
				*liquid = nullptr;
				*vision = nullptr;
			} else {
				delete new_model;
				*collision = nullptr;
				*liquid = nullptr;
				*vision = nullptr;
			}
		}
		else {
			fclose(f);
			*collision = nullptr;
			*liquid = nullptr;
			*vision = nullptr;
		}
	} else {
		*collision = nullptr;
		*liquid = nullptr;
		*vision = nullptr;
	}
}