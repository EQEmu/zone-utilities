#include "map.h"
#include <map>
#include <tuple>
#include <sstream>
#include "compression.h"

Map::Map() {
}

Map::~Map() {
}

bool Map::Build(std::string zone_name) {
	//try to load a v1-3 eqg here
	EQGLoader eqg;
	std::vector<std::shared_ptr<EQG::Geometry>> eqg_models;
	std::vector<std::shared_ptr<Placeable>> eqg_placables;
	std::vector<std::shared_ptr<EQG::Region>> eqg_regions;
	std::vector<std::shared_ptr<Light>> eqg_lights;
	if (eqg.Load(zone_name, eqg_models, eqg_placables, eqg_regions, eqg_lights)) {
		return CompileEQG(eqg_models, eqg_placables, eqg_regions, eqg_lights);
	}

	//if that fails try to load a v4 eqg here
	
	//if that fails try to load a s3d here
	S3DLoader s3d;
	std::vector<WLDFragment> zone_frags;
	std::vector<WLDFragment> zone_object_frags;
	std::vector<WLDFragment> zone_light_frags;
	std::vector<WLDFragment> object_frags;
	std::vector<WLDFragment> character_frags;
	if(s3d.Load(zone_name, zone_frags, zone_object_frags, zone_light_frags, object_frags, character_frags)) {
		return CompileS3D(zone_frags, zone_object_frags, zone_light_frags, object_frags, character_frags);
	}

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
	
	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	uint32_t collide_vert_count = (uint32_t)collide_verts.size();
	uint32_t collide_ind_count = (uint32_t)collide_indices.size();
	uint32_t non_collide_vert_count = (uint32_t)non_collide_verts.size();
	uint32_t non_collide_ind_count = (uint32_t)non_collide_indices.size();
	
	ss.write((const char*)&collide_vert_count, sizeof(uint32_t));
	ss.write((const char*)&collide_ind_count, sizeof(uint32_t));
	ss.write((const char*)&non_collide_vert_count, sizeof(uint32_t));
	ss.write((const char*)&non_collide_ind_count, sizeof(uint32_t));
	
	for(uint32_t i = 0; i < collide_vert_count; ++i) {
		auto vert = collide_verts[i];
	
		ss.write((const char*)&vert.x, sizeof(float));
		ss.write((const char*)&vert.y, sizeof(float));
		ss.write((const char*)&vert.z, sizeof(float));
	}
	
	for (uint32_t i = 0; i < collide_ind_count; ++i) {
		uint32_t ind = collide_indices[i];
	
		ss.write((const char*)&ind, sizeof(uint32_t));
	}
	
	for(uint32_t i = 0; i < non_collide_vert_count; ++i) {
		auto vert = non_collide_verts[i];
	
		ss.write((const char*)&vert.x, sizeof(float));
		ss.write((const char*)&vert.y, sizeof(float));
		ss.write((const char*)&vert.z, sizeof(float));
	}
	
	for (uint32_t i = 0; i < non_collide_ind_count; ++i) {
		uint32_t ind = non_collide_indices[i];
	
		ss.write((const char*)&ind, sizeof(uint32_t));
	}
	
	std::vector<char> buffer;
	uint32_t buffer_len = (uint32_t)(ss.str().length() + 128);
	buffer.resize(buffer_len);

	uint32_t out_size = DeflateData(ss.str().c_str(), (uint32_t)ss.str().length(), &buffer[0], buffer_len);
	if (fwrite(&out_size, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return false;
	}
	
	uint32_t uncompressed_size = (uint32_t)ss.str().length();
	if (fwrite(&uncompressed_size, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return false;
	}


	if (fwrite(&buffer[0], out_size, 1, f) != 1) {
		fclose(f);
		return false;
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
	collide_verts.clear();
	collide_indices.clear();
	non_collide_verts.clear();
	non_collide_indices.clear();

	uint32_t current_collide_index = 0;
	uint32_t current_non_collide_index = 0;
	std::map<std::tuple<float, float, float>, uint32_t> collide_vert_to_index;
	std::map<std::tuple<float, float, float>, uint32_t> non_collide_vert_to_index;
	for(uint32_t i = 0; i < zone_frags.size(); ++i) {
		if(zone_frags[i].type == 0x36) {
			WLDFragment36 &frag = reinterpret_cast<WLDFragment36&>(zone_frags[i]);
			auto model = frag.GetData();
		
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
				if(current_poly.flags == 0x10) {
					std::tuple<float, float, float> tt = std::make_tuple(v1.pos.x, v1.pos.y, v1.pos.z);
					auto iter = non_collide_vert_to_index.find(tt);
					if (iter == non_collide_vert_to_index.end()) {
						non_collide_vert_to_index[tt] = current_non_collide_index;
						non_collide_verts.push_back(v1.pos);
						non_collide_indices.push_back(current_non_collide_index);
					
						++current_non_collide_index;
					} else {
						uint32_t t_idx = iter->second;
						non_collide_indices.push_back(t_idx);
					}
					
					tt = std::make_tuple(v2.pos.x, v2.pos.y, v2.pos.z);
					iter = non_collide_vert_to_index.find(tt);
					if (iter == non_collide_vert_to_index.end()) {
						non_collide_vert_to_index[tt] = current_non_collide_index;
						non_collide_verts.push_back(v2.pos);
						non_collide_indices.push_back(current_non_collide_index);
					
						++current_non_collide_index;
					}
					else {
						uint32_t t_idx = iter->second;
						non_collide_indices.push_back(t_idx);
					}
					
					tt = std::make_tuple(v3.pos.x, v3.pos.y, v3.pos.z);
					iter = non_collide_vert_to_index.find(tt);
					if (iter == non_collide_vert_to_index.end()) {
						non_collide_vert_to_index[tt] = current_non_collide_index;
						non_collide_verts.push_back(v3.pos);
						non_collide_indices.push_back(current_non_collide_index);
					
						++current_non_collide_index;
					}
					else {
						uint32_t t_idx = iter->second;
						non_collide_indices.push_back(t_idx);
					}
				} else {
					std::tuple<float, float, float> tt = std::make_tuple(v1.pos.x, v1.pos.y, v1.pos.z);
					auto iter = collide_vert_to_index.find(tt);
					if (iter == collide_vert_to_index.end()) {
						collide_vert_to_index[tt] = current_collide_index;
						collide_verts.push_back(v1.pos);
						collide_indices.push_back(current_collide_index);

						++current_collide_index;
					}
					else {
						uint32_t t_idx = iter->second;
						collide_indices.push_back(t_idx);
					}

					tt = std::make_tuple(v2.pos.x, v2.pos.y, v2.pos.z);
					iter = collide_vert_to_index.find(tt);
					if (iter == collide_vert_to_index.end()) {
						collide_vert_to_index[tt] = current_collide_index;
						collide_verts.push_back(v2.pos);
						collide_indices.push_back(current_collide_index);

						++current_collide_index;
					}
					else {
						uint32_t t_idx = iter->second;
						collide_indices.push_back(t_idx);
					}

					tt = std::make_tuple(v3.pos.x, v3.pos.y, v3.pos.z);
					iter = collide_vert_to_index.find(tt);
					if (iter == collide_vert_to_index.end()) {
						collide_vert_to_index[tt] = current_collide_index;
						collide_verts.push_back(v3.pos);
						collide_indices.push_back(current_collide_index);

						++current_collide_index;
					}
					else {
						uint32_t t_idx = iter->second;
						collide_indices.push_back(t_idx);
					}
				}
			}
		}
	}

	std::vector<std::pair<std::shared_ptr<Placeable>, std::shared_ptr<Geometry>>> placables;
	for (uint32_t i = 0; i < zone_object_frags.size(); ++i) {
		if (zone_object_frags[i].type == 0x15) {
			WLDFragment15 &frag = reinterpret_cast<WLDFragment15&>(zone_object_frags[i]);
			auto plac = frag.GetData();
			if (plac->GetName().size() > 9) {
				std::string placable_model_name = plac->GetName().substr(0, plac->GetName().length() - 9);
				
				for (uint32_t o = 0; o < object_frags.size(); ++o) {
					if (object_frags[o].type == 0x36) {
						WLDFragment36 &obj_frag = reinterpret_cast<WLDFragment36&>(object_frags[o]);
						auto mod = obj_frag.GetData();
						if(mod->GetName().size() > 12) {
							std::string model_name = mod->GetName().substr(0, mod->GetName().length() - 12);
							if(model_name.compare(placable_model_name) == 0) {
								placables.push_back(std::make_pair(plac, mod));
								break;
							}
						}
					}
				}
			}
		}
	}

	size_t pl_sz = placables.size();
	for(size_t i = 0; i < pl_sz; ++i) {
		auto plac = placables[i].first;
		auto model = placables[i].second;

		auto &mod_polys = model->GetPolygons();
		auto &mod_verts = model->GetVertices();

		float offset_x = plac->GetX();
		float offset_y = plac->GetY();
		float offset_z = plac->GetZ();

		float rot_x = plac->GetRotateX() * 3.14159f / 180.0f;
		float rot_y = plac->GetRotateY() * 3.14159f / 180.0f;
		float rot_z = plac->GetRotateZ() * 3.14159f / 180.0f;

		float scale_x = plac->GetScaleX();
		float scale_y = plac->GetScaleY();
		float scale_z = plac->GetScaleZ();
		
		for (uint32_t j = 0; j < mod_polys.size(); ++j) {
			auto &current_poly = mod_polys[j];
			auto v1 = mod_verts[current_poly.verts[0]];
			auto v2 = mod_verts[current_poly.verts[1]];
			auto v3 = mod_verts[current_poly.verts[2]];

			RotateVertex(v1, rot_x, rot_y, rot_z);
			RotateVertex(v2, rot_x, rot_y, rot_z);
			RotateVertex(v3, rot_x, rot_y, rot_z);

			ScaleVertex(v1, scale_x, scale_y, scale_z);
			ScaleVertex(v2, scale_x, scale_y, scale_z);
			ScaleVertex(v3, scale_x, scale_y, scale_z);

			TranslateVertex(v1, offset_x, offset_y, offset_z);
			TranslateVertex(v2, offset_x, offset_y, offset_z);
			TranslateVertex(v3, offset_x, offset_y, offset_z);

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
			if (current_poly.flags == 0x10) {
				std::tuple<float, float, float> tt = std::make_tuple(v1.pos.x, v1.pos.y, v1.pos.z);
				auto iter = non_collide_vert_to_index.find(tt);
				if (iter == non_collide_vert_to_index.end()) {
					non_collide_vert_to_index[tt] = current_non_collide_index;
					non_collide_verts.push_back(v1.pos);
					non_collide_indices.push_back(current_non_collide_index);
		
					++current_non_collide_index;
				}
				else {
					uint32_t t_idx = iter->second;
					non_collide_indices.push_back(t_idx);
				}
		
				tt = std::make_tuple(v2.pos.x, v2.pos.y, v2.pos.z);
				iter = non_collide_vert_to_index.find(tt);
				if (iter == non_collide_vert_to_index.end()) {
					non_collide_vert_to_index[tt] = current_non_collide_index;
					non_collide_verts.push_back(v2.pos);
					non_collide_indices.push_back(current_non_collide_index);
		
					++current_non_collide_index;
				}
				else {
					uint32_t t_idx = iter->second;
					non_collide_indices.push_back(t_idx);
				}
		
				tt = std::make_tuple(v3.pos.x, v3.pos.y, v3.pos.z);
				iter = non_collide_vert_to_index.find(tt);
				if (iter == non_collide_vert_to_index.end()) {
					non_collide_vert_to_index[tt] = current_non_collide_index;
					non_collide_verts.push_back(v3.pos);
					non_collide_indices.push_back(current_non_collide_index);
		
					++current_non_collide_index;
				}
				else {
					uint32_t t_idx = iter->second;
					non_collide_indices.push_back(t_idx);
				}
			}
			else {
				std::tuple<float, float, float> tt = std::make_tuple(v1.pos.x, v1.pos.y, v1.pos.z);
				auto iter = collide_vert_to_index.find(tt);
				if (iter == collide_vert_to_index.end()) {
					collide_vert_to_index[tt] = current_collide_index;
					collide_verts.push_back(v1.pos);
					collide_indices.push_back(current_collide_index);
		
					++current_collide_index;
				}
				else {
					uint32_t t_idx = iter->second;
					collide_indices.push_back(t_idx);
				}
		
				tt = std::make_tuple(v2.pos.x, v2.pos.y, v2.pos.z);
				iter = collide_vert_to_index.find(tt);
				if (iter == collide_vert_to_index.end()) {
					collide_vert_to_index[tt] = current_collide_index;
					collide_verts.push_back(v2.pos);
					collide_indices.push_back(current_collide_index);
		
					++current_collide_index;
				}
				else {
					uint32_t t_idx = iter->second;
					collide_indices.push_back(t_idx);
				}
		
				tt = std::make_tuple(v3.pos.x, v3.pos.y, v3.pos.z);
				iter = collide_vert_to_index.find(tt);
				if (iter == collide_vert_to_index.end()) {
					collide_vert_to_index[tt] = current_collide_index;
					collide_verts.push_back(v3.pos);
					collide_indices.push_back(current_collide_index);
		
					++current_collide_index;
				}
				else {
					uint32_t t_idx = iter->second;
					collide_indices.push_back(t_idx);
				}
			}
		}
	}

	return true;
}

bool Map::CompileEQG(
	std::vector<std::shared_ptr<EQG::Geometry>> &models,
	std::vector<std::shared_ptr<Placeable>> &placeables,
	std::vector<std::shared_ptr<EQG::Region>> &regions,
	std::vector<std::shared_ptr<Light>> &lights
	)
{
	return false;
}

void Map::RotateVertex(Geometry::Vertex &v, float rx, float ry, float rz) {
	glm::vec3 nv = v.pos;

	nv.y = (cos(rx) * v.pos.y) - (sin(rx) * v.pos.z);
	nv.z = (sin(rx) * v.pos.y) + (cos(rx) * v.pos.z);

	v.pos = nv;
	
	nv.x = (cos(ry) * v.pos.x) + (sin(ry) * v.pos.z);
	nv.z = -(sin(ry) * v.pos.x) + (cos(ry) * v.pos.z);

	v.pos = nv;

	nv.x = (cos(rz) * v.pos.x) - (sin(rz) * v.pos.y);
	nv.y = (sin(rz) * v.pos.x) + (cos(rz) * v.pos.y);

	v.pos = nv;
}

void Map::ScaleVertex(Geometry::Vertex &v, float sx, float sy, float sz) {
	v.pos.x = v.pos.x * sx;
	v.pos.y = v.pos.y * sy;
	v.pos.z = v.pos.z * sz;
}

void Map::TranslateVertex(Geometry::Vertex &v, float tx, float ty, float tz) {
	v.pos.x = v.pos.x + tx;
	v.pos.y = v.pos.y + ty;
	v.pos.z = v.pos.z + tz;
}
