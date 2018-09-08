#include "map.h"
#include <sstream>
#include <fstream>
#include "compression.h"
#include <gtc/matrix_transform.hpp>
#include "dependency/container.h"

Map::Map() {
	_logger = EQEmu::Container::Get().Resolve<EQEmu::ILogger>();
}

Map::~Map() {
}

bool Map::Build(std::string zone_name, bool ignore_collide_tex) {
	LoadIgnore(zone_name);

	_logger->LogTrace("Attempting to load {0}.eqg as a standard eqg.", zone_name);
	
	EQEmu::EQGLoader eqg;
	std::vector<std::shared_ptr<EQEmu::EQG::Geometry>> eqg_models;
	std::vector<std::shared_ptr<EQEmu::Placeable>> eqg_placables;
	std::vector<std::shared_ptr<EQEmu::EQG::Region>> eqg_regions;
	std::vector<std::shared_ptr<EQEmu::Light>> eqg_lights;
	if (eqg.Load(zone_name, eqg_models, eqg_placables, eqg_regions, eqg_lights)) {
		return CompileEQG(eqg_models, eqg_placables, eqg_regions, eqg_lights);
	}

	_logger->LogTrace("Attempting to load {0}.eqg as a v4 eqg.", zone_name);
	EQEmu::EQG4Loader eqg4;
	if (eqg4.Load(zone_name, terrain)) {
		return CompileEQGv4();
	}
	
	_logger->LogTrace("Attempting to load {0}.s3d as a standard s3d.", zone_name);
	EQEmu::S3DLoader s3d;
	std::vector<EQEmu::S3D::WLDFragment> zone_frags;
	std::vector<EQEmu::S3D::WLDFragment> zone_object_frags;
	std::vector<EQEmu::S3D::WLDFragment> object_frags;
	if (!s3d.ParseWLDFile(zone_name + ".s3d", zone_name + ".wld", zone_frags)) {
		return false;
	}

	if (!s3d.ParseWLDFile(zone_name + ".s3d", "objects.wld", zone_object_frags)) {
		return false;
	}

	if (!s3d.ParseWLDFile(zone_name + "_obj.s3d", zone_name + "_obj.wld", object_frags)) {
		return false;
	}

	return CompileS3D(zone_frags, zone_object_frags, object_frags, ignore_collide_tex);
}

bool Map::Write(std::string filename) {
	//if there are no verts and no terrain
	if ((collide_verts.size() == 0 && collide_indices.size() == 0 && non_collide_verts.size() == 0 && non_collide_indices.size() == 0) && !terrain) {
		_logger->LogError("Failed to write {0} because the map to build has no information to write.", filename);
		return false;
	}

	FILE *f = fopen(filename.c_str(), "wb");

	if(!f) {
		_logger->LogError("Failed to write {0} because the file could not be opened to write.", filename);
		return false;
	}
	
	uint32_t version = 0x02000000;
	if (fwrite(&version, sizeof(uint32_t), 1, f) != 1) {
		_logger->LogError("Failed to write {0} because the version header could not be written.", filename);
		fclose(f);
		return false;
	}
	
	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	uint32_t collide_vert_count = (uint32_t)collide_verts.size();
	uint32_t collide_ind_count = (uint32_t)collide_indices.size();
	uint32_t non_collide_vert_count = (uint32_t)non_collide_verts.size();
	uint32_t non_collide_ind_count = (uint32_t)non_collide_indices.size();
	uint32_t model_count = (uint32_t)(map_models.size() + map_eqg_models.size());
	uint32_t plac_count = (uint32_t)map_placeables.size();
	uint32_t plac_group_count = (uint32_t)map_group_placeables.size();
	uint32_t tile_count = terrain ? (uint32_t)terrain->GetTiles().size() : 0;
	uint32_t quads_per_tile = terrain ? terrain->GetQuadsPerTile() : 0;
	float units_per_vertex = terrain ? terrain->GetUnitsPerVertex() : 0.0f;
	
	ss.write((const char*)&collide_vert_count, sizeof(uint32_t));
	ss.write((const char*)&collide_ind_count, sizeof(uint32_t));
	ss.write((const char*)&non_collide_vert_count, sizeof(uint32_t));
	ss.write((const char*)&non_collide_ind_count, sizeof(uint32_t));
	ss.write((const char*)&model_count, sizeof(uint32_t));
	ss.write((const char*)&plac_count, sizeof(uint32_t));
	ss.write((const char*)&plac_group_count, sizeof(uint32_t));
	ss.write((const char*)&tile_count, sizeof(uint32_t));
	ss.write((const char*)&quads_per_tile, sizeof(uint32_t));
	ss.write((const char*)&units_per_vertex, sizeof(float));

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

	auto model_iter = map_models.begin();
	while(model_iter != map_models.end()) {
		uint8_t null = 0;

		ss.write(model_iter->second->GetName().c_str(), model_iter->second->GetName().length());
		ss.write((const char*)&null, sizeof(uint8_t));

		auto &verts = model_iter->second->GetVertices();
		auto &polys = model_iter->second->GetPolygons();
		uint32_t vert_count = (uint32_t)verts.size();
		uint32_t poly_count = (uint32_t)polys.size();

		auto textureBrushSet = model_iter->second->GetTextureBrushSet();
		auto textureSet = textureBrushSet->GetTextureSet();
		for (auto &set : textureSet) {
			_logger->LogTrace("Texture set for model {0} with flag {1}", model_iter->second->GetName(), set->GetFlags());
		} 

		ss.write((const char*)&vert_count, sizeof(uint32_t));
		ss.write((const char*)&poly_count, sizeof(uint32_t));
		for(uint32_t i = 0; i < vert_count; ++i) {
			auto &vert = verts[i];
			float x = vert.pos.x;
			float y = vert.pos.y;
			float z = vert.pos.z;
			ss.write((const char*)&x, sizeof(float));
			ss.write((const char*)&y, sizeof(float));
			ss.write((const char*)&z, sizeof(float));
		}

		for (uint32_t i = 0; i < poly_count; ++i) {
			auto &poly = polys[i];
			uint32_t v1 = poly.verts[0];
			uint32_t v2 = poly.verts[1];
			uint32_t v3 = poly.verts[2];
			uint8_t vis = poly.flags == 0x10 ? 0 : 1;
			
			if (poly.tex < textureSet.size()) {
				_logger->LogTrace("Poly with texture {0}", poly.tex);
				auto texture = textureSet[poly.tex];
				if (texture->GetFlags() == 1) {
					vis = 0;
				}
			}

			ss.write((const char*)&v1, sizeof(uint32_t));
			ss.write((const char*)&v2, sizeof(uint32_t));
			ss.write((const char*)&v3, sizeof(uint32_t));
			ss.write((const char*)&vis, sizeof(uint8_t));
		}

		++model_iter;
	}

	auto eqg_model_iter = map_eqg_models.begin();
	while (eqg_model_iter != map_eqg_models.end()) {
		uint8_t null = 0;

		ss.write(eqg_model_iter->second->GetName().c_str(), eqg_model_iter->second->GetName().length());
		ss.write((const char*)&null, sizeof(uint8_t));

		auto &verts = eqg_model_iter->second->GetVertices();
		auto &polys = eqg_model_iter->second->GetPolygons();
		uint32_t vert_count = (uint32_t)verts.size();
		uint32_t poly_count = (uint32_t)polys.size();

		ss.write((const char*)&vert_count, sizeof(uint32_t));
		ss.write((const char*)&poly_count, sizeof(uint32_t));
		for (uint32_t i = 0; i < vert_count; ++i) {
			auto &vert = verts[i];
			float x = vert.pos.x;
			float y = vert.pos.y;
			float z = vert.pos.z;
			ss.write((const char*)&x, sizeof(float));
			ss.write((const char*)&y, sizeof(float));
			ss.write((const char*)&z, sizeof(float));
		}

		for (uint32_t i = 0; i < poly_count; ++i) {
			auto &poly = polys[i];
			uint32_t v1 = poly.verts[0];
			uint32_t v2 = poly.verts[1];
			uint32_t v3 = poly.verts[2];

			uint8_t vis = 1;
			if (poly.flags & 0x01)
				vis = 0;

			ss.write((const char*)&v1, sizeof(uint32_t));
			ss.write((const char*)&v2, sizeof(uint32_t));
			ss.write((const char*)&v3, sizeof(uint32_t));
			ss.write((const char*)&vis, sizeof(uint8_t));
		}

		++eqg_model_iter;
	}

	for (uint32_t i = 0; i < plac_count; ++i) {
		auto &plac = map_placeables[i];
		uint8_t null = 0;
		float x = plac->GetX();
		float y = plac->GetY();
		float z = plac->GetZ();
		float x_rot = plac->GetRotateX();
		float y_rot = plac->GetRotateY();
		float z_rot = plac->GetRotateZ();
		float x_scale = plac->GetScaleX();
		float y_scale = plac->GetScaleY();
		float z_scale = plac->GetScaleZ();

		ss.write(plac->GetFileName().c_str(), plac->GetFileName().length());
		ss.write((const char*)&null, sizeof(uint8_t));
		ss.write((const char*)&x, sizeof(float));
		ss.write((const char*)&y, sizeof(float));
		ss.write((const char*)&z, sizeof(float));
		ss.write((const char*)&x_rot, sizeof(float));
		ss.write((const char*)&y_rot, sizeof(float));
		ss.write((const char*)&z_rot, sizeof(float));
		ss.write((const char*)&x_scale, sizeof(float));
		ss.write((const char*)&y_scale, sizeof(float));
		ss.write((const char*)&z_scale, sizeof(float));
	}

	for (uint32_t i = 0; i < plac_group_count; ++i) {
		auto &gp = map_group_placeables[i];
		uint8_t null = 0;
		float x = gp->GetX();
		float y = gp->GetY();
		float z = gp->GetZ();
		float x_rot = gp->GetRotationX();
		float y_rot = gp->GetRotationY();
		float z_rot = gp->GetRotationZ();
		float x_scale = gp->GetScaleX();
		float y_scale = gp->GetScaleY();
		float z_scale = gp->GetScaleZ();
		float x_tile = gp->GetTileX();
		float y_tile = gp->GetTileY();
		float z_tile = gp->GetTileZ();

		ss.write((const char*)&x, sizeof(float));
		ss.write((const char*)&y, sizeof(float));
		ss.write((const char*)&z, sizeof(float));
		ss.write((const char*)&x_rot, sizeof(float));
		ss.write((const char*)&y_rot, sizeof(float));
		ss.write((const char*)&z_rot, sizeof(float));
		ss.write((const char*)&x_scale, sizeof(float));
		ss.write((const char*)&y_scale, sizeof(float));
		ss.write((const char*)&z_scale, sizeof(float));
		ss.write((const char*)&x_tile, sizeof(float));
		ss.write((const char*)&y_tile, sizeof(float));
		ss.write((const char*)&z_tile, sizeof(float));

		auto &placs = gp->GetPlaceables();
		uint32_t plac_count = (uint32_t)placs.size();
		ss.write((const char*)&plac_count, sizeof(uint32_t));
		
		for (uint32_t j = 0; j < plac_count; ++j) {
			auto &plac = placs[j];
			float x = plac->GetX();
			float y = plac->GetY();
			float z = plac->GetZ();
			float x_rot = plac->GetRotateX();
			float y_rot = plac->GetRotateY();
			float z_rot = plac->GetRotateZ();
			float x_scale = plac->GetScaleX();
			float y_scale = plac->GetScaleY();
			float z_scale = plac->GetScaleZ();

			ss.write(plac->GetFileName().c_str(), plac->GetFileName().length());
			ss.write((const char*)&null, sizeof(uint8_t));
			ss.write((const char*)&x, sizeof(float));
			ss.write((const char*)&y, sizeof(float));
			ss.write((const char*)&z, sizeof(float));
			ss.write((const char*)&x_rot, sizeof(float));
			ss.write((const char*)&y_rot, sizeof(float));
			ss.write((const char*)&z_rot, sizeof(float));
			ss.write((const char*)&x_scale, sizeof(float));
			ss.write((const char*)&y_scale, sizeof(float));
			ss.write((const char*)&z_scale, sizeof(float));
		}
	}

	if(terrain) {
		uint32_t quad_count = (quads_per_tile * quads_per_tile);
		uint32_t vert_count = ((quads_per_tile + 1) * (quads_per_tile + 1));
		auto &tiles = terrain->GetTiles();
		for (uint32_t i = 0; i < tile_count; ++i) {
			if(tiles[i]->IsFlat()) {
				bool flat = true;
				float x = tiles[i]->GetX();
				float y = tiles[i]->GetY();
				float z = tiles[i]->GetFloats()[0];
				ss.write((const char*)&flat, sizeof(bool));
				ss.write((const char*)&x, sizeof(float));
				ss.write((const char*)&y, sizeof(float));
				ss.write((const char*)&z, sizeof(float));
			} else {
				bool flat = false;
				float x = tiles[i]->GetX();
				float y = tiles[i]->GetY();
				ss.write((const char*)&flat, sizeof(bool));
				ss.write((const char*)&x, sizeof(float));
				ss.write((const char*)&y, sizeof(float));

				auto &flags = tiles[i]->GetFlags();
				for(uint32_t j = 0; j < quad_count; ++j) {
					uint8_t f = flags[j];
					ss.write((const char*)&f, sizeof(uint8_t));
				}

				auto &floats = tiles[i]->GetFloats();
				for (uint32_t j = 0; j < vert_count; ++j) {
					float f = floats[j];
					ss.write((const char*)&f, sizeof(float));
				}
			}
		}
	}
	
	std::vector<char> buffer;
	uint32_t buffer_len = (uint32_t)(ss.str().length() + 128);
	buffer.resize(buffer_len);

	uint32_t out_size = EQEmu::DeflateData(ss.str().c_str(), (uint32_t)ss.str().length(), &buffer[0], buffer_len);
	if (fwrite(&out_size, sizeof(uint32_t), 1, f) != 1) {
		_logger->LogError("Failed to write {0} because the compressed size header could not be written.", filename);
		fclose(f);
		return false;
	}
	
	uint32_t uncompressed_size = (uint32_t)ss.str().length();
	if (fwrite(&uncompressed_size, sizeof(uint32_t), 1, f) != 1) {
		_logger->LogError("Failed to write {0} because the uncompressed size header could not be written.", filename);
		fclose(f);
		return false;
	}


	if (fwrite(&buffer[0], out_size, 1, f) != 1) {
		_logger->LogError("Failed to write {0} because the compressed data could not be written.", filename);
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}

void Map::TraverseBone(std::shared_ptr<EQEmu::S3D::SkeletonTrack::Bone> bone, glm::vec3 parent_trans, glm::vec3 parent_rot, glm::vec3 parent_scale)
{
	float offset_x = 0.0f;
	float offset_y = 0.0f;
	float offset_z = 0.0f;

	float rot_x = 0.0f;
	float rot_y = 0.0f;
	float rot_z = 0.0f;

	float scale_x = parent_scale.x;
	float scale_y = parent_scale.y;
	float scale_z = parent_scale.z;

	if(bone->orientation) {
		if (bone->orientation->shift_denom != 0) {
			offset_x = (float)bone->orientation->shift_x_num / (float)bone->orientation->shift_denom;
			offset_y = (float)bone->orientation->shift_y_num / (float)bone->orientation->shift_denom;
			offset_z = (float)bone->orientation->shift_z_num / (float)bone->orientation->shift_denom;
		}

		if(bone->orientation->rotate_denom != 0) {
			rot_x = (float)bone->orientation->rotate_x_num / (float)bone->orientation->rotate_denom;
			rot_y = (float)bone->orientation->rotate_y_num / (float)bone->orientation->rotate_denom;
			rot_z = (float)bone->orientation->rotate_z_num / (float)bone->orientation->rotate_denom;

			rot_x = rot_x * 3.14159f / 180.0f;
			rot_y = rot_y * 3.14159f / 180.0f;
			rot_z = rot_z * 3.14159f / 180.0f;
		}
	}

	glm::vec3 pos(offset_x, offset_y, offset_z);
	glm::vec3 rot(rot_x, rot_y, rot_z);

	RotateVertex(pos, parent_rot.x, parent_rot.y, parent_rot.z);
	pos += parent_trans;
	
	rot += parent_rot;

	if(bone->model) {
		auto &mod_polys = bone->model->GetPolygons();
		auto &mod_verts = bone->model->GetVertices();

		if (map_models.count(bone->model->GetName()) == 0) {
			map_models[bone->model->GetName()] = bone->model;
		}
		
		std::shared_ptr<EQEmu::Placeable> gen_plac(new EQEmu::Placeable());
		gen_plac->SetFileName(bone->model->GetName());
		gen_plac->SetLocation(pos.x, pos.y, pos.z);
		gen_plac->SetRotation(rot.x, rot.y, rot.z);
		gen_plac->SetScale(scale_x, scale_y, scale_z);
		map_placeables.push_back(gen_plac);

		_logger->LogTrace("Adding placeable {0} at ({1}, {2}, {3})",bone->model->GetName(), pos.x, pos.y, pos.z);
	}

	for(size_t i = 0; i < bone->children.size(); ++i) {
		TraverseBone(bone->children[i], pos, rot, parent_scale);
	}
}

bool Map::CompileS3D(
	std::vector<EQEmu::S3D::WLDFragment> &zone_frags,
	std::vector<EQEmu::S3D::WLDFragment> &zone_object_frags,
	std::vector<EQEmu::S3D::WLDFragment> &object_frags,
	bool ignore_collide_tex
	)
{
	collide_verts.clear();
	collide_indices.clear();
	non_collide_verts.clear();
	non_collide_indices.clear();
	current_collide_index = 0;
	current_non_collide_index = 0;
	collide_vert_to_index.clear();
	non_collide_vert_to_index.clear();
	map_models.clear();
	map_eqg_models.clear();
	map_placeables.clear();

	_logger->LogTrace("Processing s3d zone geometry fragments.");
	for(uint32_t i = 0; i < zone_frags.size(); ++i) {
		if(zone_frags[i].type == 0x36) {
			EQEmu::S3D::WLDFragment36 &frag = reinterpret_cast<EQEmu::S3D::WLDFragment36&>(zone_frags[i]);
			auto model = frag.GetData();
			auto &mod_polys = model->GetPolygons();
			auto &mod_verts = model->GetVertices();

			for (uint32_t j = 0; j < mod_polys.size(); ++j) {
				auto &current_poly = mod_polys[j];
                bool collide_tex = false;

                auto tbs = model->GetTextureBrushSet();
                if (current_poly.tex >= 0 && current_poly.tex < tbs->GetTextureSet().size()) {
                    auto &tex = tbs->GetTextureSet()[current_poly.tex];
                    auto &textures = tex->GetTextures();
                    if (textures.size() == 1) {
                        auto &t = textures[0];
                        auto &frames = t->GetTextureFrames();

                        if (frames.size() == 1) {
                            auto &f = frames[0];

                            if (f.compare("collide.dds") == 0) {
                                collide_tex = true;
                            }
                        }
                    }
                }

				auto v1 = mod_verts[current_poly.verts[0]];
				auto v2 = mod_verts[current_poly.verts[1]];
				auto v3 = mod_verts[current_poly.verts[2]];

				float t = v1.pos.x;
				v1.pos.x = v1.pos.y;
				v1.pos.y = t;

				t = v2.pos.x;
				v2.pos.x = v2.pos.y;
				v2.pos.y = t;

				t = v3.pos.x;
				v3.pos.x = v3.pos.y;
				v3.pos.y = t;

				if(current_poly.flags == 0x10)
					AddFace(v1.pos, v2.pos, v3.pos, false);
                else {
                    if (!collide_tex || !ignore_collide_tex) {
                        AddFace(v1.pos, v2.pos, v3.pos, true);
                    }
                }
			}
		}
	}

	_logger->LogTrace("Processing zone placeable fragments.");
	std::vector<std::pair<std::shared_ptr<EQEmu::Placeable>, std::shared_ptr<EQEmu::S3D::Geometry>>> placables;
	std::vector<std::pair<std::shared_ptr<EQEmu::Placeable>, std::shared_ptr<EQEmu::S3D::SkeletonTrack>>> placables_skeleton;
	for (uint32_t i = 0; i < zone_object_frags.size(); ++i) {
		if (zone_object_frags[i].type == 0x15) {
			EQEmu::S3D::WLDFragment15 &frag = reinterpret_cast<EQEmu::S3D::WLDFragment15&>(zone_object_frags[i]);
			auto plac = frag.GetData();

			if(!plac)
			{
				_logger->LogWarning("Placeable entry was not found.");
				continue;
			}

			if (ignore_placs.count(plac->GetName()) > 0) {
				continue;
			}

			_logger->LogTrace("Loading placeable {0}", plac->GetName());
			bool found = false;
			for (uint32_t o = 0; o < object_frags.size(); ++o) {
				if (object_frags[o].type == 0x14) {
					EQEmu::S3D::WLDFragment14 &obj_frag = reinterpret_cast<EQEmu::S3D::WLDFragment14&>(object_frags[o]);
					auto mod_ref = obj_frag.GetData();

					if(mod_ref->GetName().compare(plac->GetName()) == 0) {
						found = true;

						auto &frag_refs = mod_ref->GetFrags();
						for (uint32_t m = 0; m < frag_refs.size(); ++m) {
							if (object_frags[frag_refs[m] - 1].type == 0x2D) {
								EQEmu::S3D::WLDFragment2D &r_frag = reinterpret_cast<EQEmu::S3D::WLDFragment2D&>(object_frags[frag_refs[m] - 1]);
								auto m_ref = r_frag.GetData();

								EQEmu::S3D::WLDFragment36 &mod_frag = reinterpret_cast<EQEmu::S3D::WLDFragment36&>(object_frags[m_ref]);
								auto mod = mod_frag.GetData();
								placables.push_back(std::make_pair(plac, mod));
							}
							else if (object_frags[frag_refs[m] - 1].type == 0x11) {
								EQEmu::S3D::WLDFragment11 &r_frag = reinterpret_cast<EQEmu::S3D::WLDFragment11&>(object_frags[frag_refs[m] - 1]);
								auto s_ref = r_frag.GetData();

								EQEmu::S3D::WLDFragment10 &skeleton_frag = reinterpret_cast<EQEmu::S3D::WLDFragment10&>(object_frags[s_ref]);
								auto skele = skeleton_frag.GetData();
								
								placables_skeleton.push_back(std::make_pair(plac, skele));
							}
						}

						break;
					}
				}
			}

			if(!found) {
				_logger->LogWarning("Could not find model for placeable {0}", plac->GetName());
			}
		}
	}

	_logger->LogTrace("Processing s3d placeables.");
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
	
		if (map_models.count(model->GetName()) == 0) {
			map_models[model->GetName()] = model;
		}
		std::shared_ptr<EQEmu::Placeable> gen_plac(new EQEmu::Placeable());
		gen_plac->SetFileName(model->GetName());
		gen_plac->SetLocation(offset_x, offset_y, offset_z);
		//y rotation seems backwards on s3ds, probably cause of the weird coord system they used back then
		//x rotation might be too but there are literally 0 x rotated placeables in all the s3ds so who knows
		gen_plac->SetRotation(rot_x, -rot_y, rot_z);
		gen_plac->SetScale(scale_x, scale_y, scale_z);
		map_placeables.push_back(gen_plac);

		_logger->LogTrace("Adding placeable {0} at ({1}, {2}, {3})", model->GetName(), offset_x, offset_y, offset_z);
	}

	_logger->LogTrace("Processing s3d animated placeables.");
	pl_sz = placables_skeleton.size();
	for (size_t i = 0; i < pl_sz; ++i) {
		auto &plac = placables_skeleton[i].first;
		
		auto &bones = placables_skeleton[i].second->GetBones();

		if(bones.size() > 0) 
		{
			float offset_x = plac->GetX();
			float offset_y = plac->GetY();
			float offset_z = plac->GetZ();

			float rot_x = plac->GetRotateX() * 3.14159f / 180.0f;
			float rot_y = plac->GetRotateY() * 3.14159f / 180.0f;
			float rot_z = plac->GetRotateZ() * 3.14159f / 180.0f;

			float scale_x = plac->GetScaleX();
			float scale_y = plac->GetScaleY();
			float scale_z = plac->GetScaleZ();
			TraverseBone(bones[0], glm::vec3(offset_x, offset_y, offset_z), glm::vec3(rot_x, rot_y, rot_z), glm::vec3(scale_x, scale_y, scale_z));
		}
	}

	return true;
}

bool Map::CompileEQG(
	std::vector<std::shared_ptr<EQEmu::EQG::Geometry>> &models,
	std::vector<std::shared_ptr<EQEmu::Placeable>> &placeables,
	std::vector<std::shared_ptr<EQEmu::EQG::Region>> &regions,
	std::vector<std::shared_ptr<EQEmu::Light>> &lights
	)
{
	collide_verts.clear();
	collide_indices.clear();
	non_collide_verts.clear();
	non_collide_indices.clear();
	current_collide_index = 0;
	current_non_collide_index = 0;
	collide_vert_to_index.clear();
	non_collide_vert_to_index.clear();
	map_models.clear();
	map_eqg_models.clear();
	map_placeables.clear();

	for(uint32_t i = 0; i < placeables.size(); ++i) {
		std::shared_ptr<EQEmu::Placeable> &plac = placeables[i];
		std::shared_ptr<EQEmu::EQG::Geometry> model;
		bool is_ter = false;

		if(plac->GetName().substr(0, 3).compare("TER") == 0 || plac->GetFileName().substr(plac->GetFileName().length() - 4, 4).compare(".ter") == 0)
			is_ter = true;

		for(uint32_t j = 0; j < models.size(); ++j) {
			if(models[j]->GetName().compare(plac->GetFileName()) == 0) {
				model = models[j];
				break;
			}
		}

		if (!model) {
			_logger->LogWarning("Could not find placeable {0}.", plac->GetFileName());
			continue;
		}

		float offset_x = plac->GetX();
		float offset_y = plac->GetY();
		float offset_z = plac->GetZ();

		float rot_x = plac->GetRotateX() * 3.14159f / 180.0f;
		float rot_y = plac->GetRotateY() * 3.14159f / 180.0f;
		float rot_z = plac->GetRotateZ() * 3.14159f / 180.0f;

		float scale_x = plac->GetScaleX();
		float scale_y = plac->GetScaleY();
		float scale_z = plac->GetScaleZ();

		if(!is_ter) {
			if (ignore_placs.count(model->GetName()) > 0) {
				continue;
			}

			if (map_eqg_models.count(model->GetName()) == 0) {
				map_eqg_models[model->GetName()] = model;
			}

			std::shared_ptr<EQEmu::Placeable> gen_plac(new EQEmu::Placeable());
			gen_plac->SetFileName(model->GetName());
			gen_plac->SetLocation(offset_x, offset_y, offset_z);
			gen_plac->SetRotation(rot_x, rot_y, rot_z);
			gen_plac->SetScale(scale_x, scale_y, scale_z);
			map_placeables.push_back(gen_plac);

			_logger->LogTrace("Adding placeable {0} at ({1}, {2}, {3})", model->GetName(), offset_x, offset_y, offset_z);
			continue;
		}

		auto &mod_polys = model->GetPolygons();
		auto &mod_verts = model->GetVertices();

		for (uint32_t j = 0; j < mod_polys.size(); ++j) {
			auto &current_poly = mod_polys[j];
			auto v1 = mod_verts[current_poly.verts[0]];
			auto v2 = mod_verts[current_poly.verts[1]];
			auto v3 = mod_verts[current_poly.verts[2]];


			float t = v1.pos.x;
			v1.pos.x = v1.pos.y;
			v1.pos.y = t;

			t = v2.pos.x;
			v2.pos.x = v2.pos.y;
			v2.pos.y = t;

			t = v3.pos.x;
			v3.pos.x = v3.pos.y;
			v3.pos.y = t;

			if (current_poly.flags & 0x01)
				AddFace(v1.pos, v2.pos, v3.pos, false);
			else
				AddFace(v1.pos, v2.pos, v3.pos, true);
		}
	}

	return true;
}

bool Map::CompileEQGv4()
{
	collide_verts.clear();
	collide_indices.clear();
	non_collide_verts.clear();
	non_collide_indices.clear();
	current_collide_index = 0;
	current_non_collide_index = 0;
	collide_vert_to_index.clear();
	non_collide_vert_to_index.clear();
	map_models.clear();
	map_eqg_models.clear();
	map_placeables.clear();

	if(!terrain)
		return false;

	auto &water_sheets = terrain->GetWaterSheets();
	for(size_t i = 0; i < water_sheets.size(); ++i) {
		auto &sheet = water_sheets[i];
	
		if(sheet->GetTile()) {
			auto &tiles = terrain->GetTiles();
			for(size_t j = 0; j < tiles.size(); ++j) {
				float x = tiles[j]->GetX();
				float y = tiles[j]->GetY();
				float z = tiles[j]->GetBaseWaterLevel();
				
				float QuadVertex1X = x;
				float QuadVertex1Y = y;
				float QuadVertex1Z = z;

				float QuadVertex2X = QuadVertex1X + (terrain->GetOpts().quads_per_tile * terrain->GetOpts().units_per_vert);
				float QuadVertex2Y = QuadVertex1Y;
				float QuadVertex2Z = QuadVertex1Z;

				float QuadVertex3X = QuadVertex2X;
				float QuadVertex3Y = QuadVertex1Y + (terrain->GetOpts().quads_per_tile * terrain->GetOpts().units_per_vert);
				float QuadVertex3Z = QuadVertex1Z;

				float QuadVertex4X = QuadVertex1X;
				float QuadVertex4Y = QuadVertex3Y;
				float QuadVertex4Z = QuadVertex1Z;

				uint32_t current_vert = (uint32_t)non_collide_verts.size() + 3;
				non_collide_verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
				non_collide_verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
				non_collide_verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
				non_collide_verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));
				
				non_collide_indices.push_back(current_vert);
				non_collide_indices.push_back(current_vert - 2);
				non_collide_indices.push_back(current_vert - 1);

				non_collide_indices.push_back(current_vert);
				non_collide_indices.push_back(current_vert - 3);
				non_collide_indices.push_back(current_vert - 2);
			}
		} else {
			uint32_t id = (uint32_t)non_collide_verts.size();

			non_collide_verts.push_back(glm::vec3(sheet->GetMinY(), sheet->GetMinX(), sheet->GetZHeight()));
			non_collide_verts.push_back(glm::vec3(sheet->GetMinY(), sheet->GetMaxX(), sheet->GetZHeight()));
			non_collide_verts.push_back(glm::vec3(sheet->GetMaxY(), sheet->GetMinX(), sheet->GetZHeight()));
			non_collide_verts.push_back(glm::vec3(sheet->GetMaxY(), sheet->GetMaxX(), sheet->GetZHeight()));

			non_collide_indices.push_back(id);
			non_collide_indices.push_back(id + 1);
			non_collide_indices.push_back(id + 2);

			non_collide_indices.push_back(id + 1);
			non_collide_indices.push_back(id + 3);
			non_collide_indices.push_back(id + 2);
		}
	}

	auto &invis_walls = terrain->GetInvisWalls();
	for (size_t i = 0; i < invis_walls.size(); ++i) {
		auto &wall = invis_walls[i];
		auto &verts = wall->GetVerts();

		for(size_t j = 0; j < verts.size(); ++j) {
			if (j + 1 == verts.size())
				break;

			float t;
			auto v1 = verts[j];
			auto v2 = verts[j + 1];

			t = v1.x;
			v1.x = v1.y;
			v1.y = t;

			t = v2.x;
			v2.x = v2.y;
			v2.y = t;

			glm::vec3 v3 = v1;
			v3.z += 1000.0;

			glm::vec3 v4 = v2;
			v4.z += 1000.0;

			AddFace(v2, v1, v3, true);
			AddFace(v3, v4, v2, true);

			AddFace(v3, v1, v2, true);
			AddFace(v2, v4, v3, true);
		}

	}

	//map_eqg_models
	auto &models = terrain->GetModels();
	auto model_iter = models.begin();
	while(model_iter != models.end()) {
		auto &model = model_iter->second;
		if (map_eqg_models.count(model->GetName()) == 0) {
			map_eqg_models[model->GetName()] = model;
		}
		++model_iter;
	}

	//map_placeables
	auto &pgs = terrain->GetPlaceableGroups();
	for(size_t i = 0; i < pgs.size(); ++i) {
		map_group_placeables.push_back(pgs[i]);
	}

	return true;
}

void Map::LoadIgnore(std::string zone_name)
{
	ignore_placs.clear();
	std::string filename = zone_name + ".ignore";

	std::ifstream f(filename, std::ifstream::in);
	char buffer[128];
	while (f.good()) {
		f.getline(buffer, 128);
		ignore_placs[buffer] = true;
	}
	f.close();
}

void Map::AddFace(glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3, bool collidable) {
	if (!collidable) {
		std::tuple<float, float, float> tt = std::make_tuple(v1.x, v1.y, v1.z);
		auto iter = non_collide_vert_to_index.find(tt);
		if (iter == non_collide_vert_to_index.end()) {
			non_collide_vert_to_index[tt] = current_non_collide_index;
			non_collide_verts.push_back(v1);
			non_collide_indices.push_back(current_non_collide_index);

			++current_non_collide_index;
		}
		else {
			uint32_t t_idx = iter->second;
			non_collide_indices.push_back(t_idx);
		}

		tt = std::make_tuple(v2.x, v2.y, v2.z);
		iter = non_collide_vert_to_index.find(tt);
		if (iter == non_collide_vert_to_index.end()) {
			non_collide_vert_to_index[tt] = current_non_collide_index;
			non_collide_verts.push_back(v2);
			non_collide_indices.push_back(current_non_collide_index);

			++current_non_collide_index;
		}
		else {
			uint32_t t_idx = iter->second;
			non_collide_indices.push_back(t_idx);
		}

		tt = std::make_tuple(v3.x, v3.y, v3.z);
		iter = non_collide_vert_to_index.find(tt);
		if (iter == non_collide_vert_to_index.end()) {
			non_collide_vert_to_index[tt] = current_non_collide_index;
			non_collide_verts.push_back(v3);
			non_collide_indices.push_back(current_non_collide_index);

			++current_non_collide_index;
		}
		else {
			uint32_t t_idx = iter->second;
			non_collide_indices.push_back(t_idx);
		}
	}
	else {
		std::tuple<float, float, float> tt = std::make_tuple(v1.x, v1.y, v1.z);
		auto iter = collide_vert_to_index.find(tt);
		if (iter == collide_vert_to_index.end()) {
			collide_vert_to_index[tt] = current_collide_index;
			collide_verts.push_back(v1);
			collide_indices.push_back(current_collide_index);

			++current_collide_index;
		}
		else {
			uint32_t t_idx = iter->second;
			collide_indices.push_back(t_idx);
		}

		tt = std::make_tuple(v2.x, v2.y, v2.z);
		iter = collide_vert_to_index.find(tt);
		if (iter == collide_vert_to_index.end()) {
			collide_vert_to_index[tt] = current_collide_index;
			collide_verts.push_back(v2);
			collide_indices.push_back(current_collide_index);

			++current_collide_index;
		}
		else {
			uint32_t t_idx = iter->second;
			collide_indices.push_back(t_idx);
		}

		tt = std::make_tuple(v3.x, v3.y, v3.z);
		iter = collide_vert_to_index.find(tt);
		if (iter == collide_vert_to_index.end()) {
			collide_vert_to_index[tt] = current_collide_index;
			collide_verts.push_back(v3);
			collide_indices.push_back(current_collide_index);

			++current_collide_index;
		}
		else {
			uint32_t t_idx = iter->second;
			collide_indices.push_back(t_idx);
		}
	}
}

void Map::RotateVertex(glm::vec3 &v, float rx, float ry, float rz) {
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

void Map::ScaleVertex(glm::vec3 &v, float sx, float sy, float sz) {
	v.x = v.x * sx;
	v.y = v.y * sy;
	v.z = v.z * sz;
}

void Map::TranslateVertex(glm::vec3 &v, float tx, float ty, float tz) {
	v.x = v.x + tx;
	v.y = v.y + ty;
	v.z = v.z + tz;
}
