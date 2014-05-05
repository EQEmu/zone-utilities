#include "map.h"
#include "s3d_loader.h"
#include <map>
#include <tuple>

Map::Map() {
}

Map::~Map() {
}

bool Map::Build(std::string zone_name) {
	S3DLoader s3d;
	std::vector<WLDFragment> zone_frags;
	std::vector<WLDFragment> zone_object_frags;
	std::vector<WLDFragment> zone_light_frags;
	std::vector<WLDFragment> object_frags;
	std::vector<WLDFragment> character_frags;
	if(s3d.Load(zone_name, zone_frags, zone_object_frags, zone_light_frags, object_frags, character_frags)) {
		return CompileS3D(zone_frags, zone_object_frags, zone_light_frags, object_frags, character_frags);
	}

	//try to load a v1-3 eqg here

	//if that fails try to load a v4 eqg here

	//all hath failed
	return false;
}

bool Map::Write(std::string filename) {
	FILE *f = fopen(filename.c_str(), "wb");

	uint32_t version = 0x02000000;
	if (fwrite(&version, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return false;
	}

	uint32_t vert_count = verts.size();
	uint32_t ind_count = indices.size();
	uint32_t flag_count = polygon_flags.size();

	if (fwrite(&vert_count, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return false;
	}

	if (fwrite(&ind_count, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return false;
	}

	if (fwrite(&flag_count, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return false;
	}

	for(uint32_t i = 0; i < vert_count; ++i) {
		auto vert = verts[i];

		if (fwrite(&(vert.x), sizeof(float), 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (fwrite(&(vert.y), sizeof(float), 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (fwrite(&(vert.z), sizeof(float), 1, f) != 1) {
			fclose(f);
			return false;
		}
	}

	for (uint32_t i = 0; i < ind_count; ++i) {
		uint32_t ind = indices[i];

		if (fwrite(&ind, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return false;
		}
	}

	for (uint32_t i = 0; i < flag_count; ++i) {
		uint32_t flag = polygon_flags[i];

		if (fwrite(&flag, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return false;
		}
	}

	fclose(f);
	return true;
}

bool Map::CompileS3D(
	std::vector<WLDFragment> &zone_frags,
	std::vector<WLDFragment> &zone_object_frags,
	std::vector<WLDFragment> &zone_light_frags,
	std::vector<WLDFragment> &object_frags,
	std::vector<WLDFragment> &character_frags
	)
{
	uint32_t idx = 0;
	std::map<std::tuple<float, float, float>, uint32_t> vert_to_index;
	for(uint32_t i = 0; i < zone_frags.size(); ++i) {
		if(zone_frags[i].type == 0x36) {
			WLDFragment36 &frag = reinterpret_cast<WLDFragment36&>(zone_frags[i]);
			auto model = frag.GetData();

			//add this model
			printf("Adding geometry fragment: %s\n", model->GetName().c_str());
			
			auto &mod_polys = model->GetPolygons();
			auto &mod_verts = model->GetVertices();

			for (uint32_t j = 0; j < mod_polys.size(); ++j) {
				auto &current_poly = mod_polys[j];
				auto v1 = mod_verts[current_poly.verts[0]];
				auto v2 = mod_verts[current_poly.verts[1]];
				auto v3 = mod_verts[current_poly.verts[2]];
#ifdef INVERSEXY
				float t = v1.pos.x;
				v1.pos.x = v1.pos.y;
				v1.pos.y = t;

				t = v2.pos.x;
				v2.pos.x = v2.pos.y;
				v2.pos.y = t;

				t = v3.pos.x;
				v3.pos.x = v3.pos.y;
				v3.pos.y = t;
#endif
				std::tuple<float, float, float> tt = std::make_tuple(v1.pos.x, v1.pos.y, v1.pos.z);
				auto iter = vert_to_index.find(tt);
				if (iter == vert_to_index.end()) {
					vert_to_index[tt] = idx;
					verts.push_back(v1.pos);
					indices.push_back(idx);
				
					++idx;
				} else {
					uint32_t t_idx = iter->second;
					indices.push_back(t_idx);
				}
				
				tt = std::make_tuple(v2.pos.x, v2.pos.y, v2.pos.z);
				iter = vert_to_index.find(tt);
				if (iter == vert_to_index.end()) {
					vert_to_index[tt] = idx;
					verts.push_back(v2.pos);
					indices.push_back(idx);
				
					++idx;
				}
				else {
					uint32_t t_idx = iter->second;
					indices.push_back(t_idx);
				}
				
				tt = std::make_tuple(v3.pos.x, v3.pos.y, v3.pos.z);
				iter = vert_to_index.find(tt);
				if (iter == vert_to_index.end()) {
					vert_to_index[tt] = idx;
					verts.push_back(v3.pos);
					indices.push_back(idx);
				
					++idx;
				}
				else {
					uint32_t t_idx = iter->second;
					indices.push_back(t_idx);
				}

				if (current_poly.flags == 0x10)
					polygon_flags.push_back(1);
				else
					polygon_flags.push_back(0);
			}
		}
	}

	printf("Loaded %u verts %u ind %u flags\n", verts.size(), indices.size(), polygon_flags.size());
	return true;
}