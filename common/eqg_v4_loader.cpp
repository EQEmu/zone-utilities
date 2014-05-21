#include "eqg_v4_loader.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include "eqg_structs.h"
#include "safe_alloc.h"
#include "eqg_model_loader.h"
#include "string_util.h"
#include "log_macros.h"

EQEmu::EQG4Loader::EQG4Loader() {
}

EQEmu::EQG4Loader::~EQG4Loader() {
}

bool EQEmu::EQG4Loader::Load(std::string file, std::shared_ptr<EQG::Terrain> &terrain)
{
	EQEmu::PFS::Archive archive;
	if (!archive.Open(file + ".eqg")) {
		eqLogMessage(LogTrace, "Failed to open %s.eqg as an eqgv4 file because the file does not exist.", file.c_str());
		return false;
	}

	std::vector<char> zon;
	bool zon_found = true;
	std::list<std::string> files;
	archive.GetFilenames("zon", files);

	if (files.size() == 0) {
		if (!GetZon(file + ".zon", zon)) {
			zon_found = false;
		}
	}
	else {
		auto iter = files.begin();
		if (!archive.Get(*iter, zon)) {
			zon_found = false;
		}
	}

	if (!zon_found) {
		eqLogMessage(LogError, "Failed to open %s.eqg because the %s.zon file could not be found.", file.c_str(), file.c_str());
		return false;
	}

	eqLogMessage(LogTrace, "Parsing zone file.");
	terrain.reset(new EQG::Terrain());
	if (!ParseZon(zon, terrain->GetOpts())) {
		return false;
	}

	eqLogMessage(LogTrace, "Parsing zone data file.");
	if(!ParseZoneDat(archive, terrain)) {
		return false;
	}

	eqLogMessage(LogTrace, "Parsing water data file.");
	ParseWaterDat(archive, terrain);

	eqLogMessage(LogTrace, "Parsing invisible walls file.");
	ParseInvwDat(archive, terrain);

	return true;
}

float HeightWithinQuad(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, float x, float y)
{
	int inTriangle = 0;

	glm::vec3 a, b, c;
	glm::vec3 n;

	float fAB = (y - p1.y) * (p2.x - p1.x) - (x - p1.x) * (p2.y - p1.y);
	float fBC = (y - p2.y) * (p3.x - p2.x) - (x - p2.x) * (p3.y - p2.y);
	float fCA = (y - p3.y) * (p1.x - p3.x) - (x - p3.x) * (p1.y - p3.y);

	if ((fAB * fBC >= 0) && (fBC * fCA >= 0))
	{
		inTriangle = 1;
		a = p1;
		b = p2;
		c = p3;
	}

	fAB = (y - p1.y) * (p3.x - p1.x) - (x - p1.x) * (p3.y - p1.y);
	fBC = (y - p3.y) * (p4.x - p3.x) - (x - p3.x) * (p4.y - p3.y);
	fCA = (y - p4.y) * (p1.x - p4.x) - (x - p4.x) * (p1.y - p4.y);


	if ((fAB * fBC >= 0) && (fBC * fCA >= 0))
	{
		inTriangle = 2;
		a = p1;
		b = p3;
		c = p4;
	}

	n.x = (b.y - a.y)*(c.z - a.z) - (b.z - a.z)*(c.y - a.y);
	n.y = (b.z - a.z)*(c.x - a.x) - (b.x - a.x)*(c.z - a.z);
	n.z = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);

	float len = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);

	n.x /= len;
	n.y /= len;
	n.z /= len;

	return (((n.x) * (x - a.x) + (n.y) * (y - a.y)) / -n.z) + a.z;
}

bool EQEmu::EQG4Loader::ParseZoneDat(EQEmu::PFS::Archive &archive, std::shared_ptr<EQG::Terrain> &terrain) {
	std::string filename = terrain->GetOpts().name + ".dat";
	std::vector<char> buffer;
	if(!archive.Get(filename, buffer)) {
		eqLogMessage(LogError, "Failed to open %s.", filename.c_str());
		return false;
	}

	uint32_t idx = 0;

	SafeVarAllocParse(v4_zone_dat_header, header);

	SafeStringAllocParse(base_tile_texture);
	SafeVarAllocParse(uint32_t, tile_count);

	float zone_min_x = (float)terrain->GetOpts().min_lat * (float)terrain->GetOpts().quads_per_tile * (float)terrain->GetOpts().units_per_vert;
	//float zone_max_x = (float)(terrain->GetOpts().max_lat + 1) * (float)terrain->GetOpts().quads_per_tile * (float)terrain->GetOpts().units_per_vert;

	float zone_min_y = (float)terrain->GetOpts().min_lng * (float)terrain->GetOpts().quads_per_tile * terrain->GetOpts().units_per_vert;
	//float zone_max_y = (float)(terrain->GetOpts().max_lng + 1) * (float)terrain->GetOpts().quads_per_tile * terrain->GetOpts().units_per_vert;

	uint32_t quad_count = (terrain->GetOpts().quads_per_tile * terrain->GetOpts().quads_per_tile);
	uint32_t vert_count = ((terrain->GetOpts().quads_per_tile + 1) * (terrain->GetOpts().quads_per_tile + 1));

	terrain->SetQuadsPerTile(terrain->GetOpts().quads_per_tile);
	terrain->SetUnitsPerVertex(terrain->GetOpts().units_per_vert);

	eqLogMessage(LogTrace, "Parsing zone terrain tiles.");
	for(uint32_t i = 0; i < tile_count; ++i) {
		std::shared_ptr<EQG::TerrainTile> tile(new EQG::TerrainTile());
		terrain->AddTile(tile);

		SafeVarAllocParse(int32_t, tile_lng);
		SafeVarAllocParse(int32_t, tile_lat);
		SafeVarAllocParse(int32_t, tile_unk);

		float tile_start_y = zone_min_y + (tile_lng - 100000 - terrain->GetOpts().min_lng) * terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile;
		float tile_start_x = zone_min_x + (tile_lat - 100000 - terrain->GetOpts().min_lat) * terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile;
	
		bool floats_all_the_same = true;
		tile->GetFloats().resize(vert_count);
		tile->GetColors().resize(vert_count);
		tile->GetColors2().resize(vert_count);
		tile->GetFlags().resize(quad_count);

		double current_avg = 0.0;
		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(float, t);
			tile->GetFloats()[j] = t;

			if ((j > 0) && (tile->GetFloats()[j] != tile->GetFloats()[0])) {
				floats_all_the_same = false;
			}
		}

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(uint32_t, color);
			tile->GetColors()[j] = color;
		}

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(uint32_t, color);
			tile->GetColors2()[j] = color;
		}

		for (uint32_t j = 0; j < quad_count; ++j) {
			SafeVarAllocParse(uint8_t, flag);
			tile->GetFlags()[j] = flag;

			if(flag & 0x01)
				floats_all_the_same = false;
		}

		if (floats_all_the_same)
			tile->SetFlat(true);

		SafeVarAllocParse(float, BaseWaterLevel);
		tile->SetBaseWaterLevel(BaseWaterLevel);

		SafeVarAllocParse(int32_t, unk_unk);
		idx -= sizeof(int32_t);
		SafeVarAllocParse(float, unk_unk2);

		if (unk_unk > 0) {
			SafeVarAllocParse(int8_t, unk_byte);
			if(unk_byte > 0) {
				SafeVarAllocParse(float, f1);
				SafeVarAllocParse(float, f2);
				SafeVarAllocParse(float, f3);
				SafeVarAllocParse(float, f4);
			}

			SafeVarAllocParse(float, f1);
		}

		SafeVarAllocParse(uint32_t, layer_count);
		SafeStringAllocParse(base_material);
		//tile.SetBaseMaterial(base_material);

		uint32_t overlay_count = 0;
		for (uint32_t layer = 1; layer < layer_count; ++layer) {
			SafeStringAllocParse(material);

			SafeVarAllocParse(uint32_t, detail_mask_dim);
			uint32_t sz_m = detail_mask_dim * detail_mask_dim;

			//TerrainTileLayer layer;
			//DetailMask dm;
			for (uint32_t b = 0; b < sz_m; ++b)
			{
				SafeVarAllocParse(uint8_t, detail_mask_byte);
				//dm.AddByte(detail_mask_byte);
			}

			//layer.AddDetailMask(dm);
			//tile.AddLayer(layer);
			++overlay_count;
		}

		SafeVarAllocParse(uint32_t, single_placeable_count);
		for(uint32_t j = 0; j < single_placeable_count; ++j) {
			SafeStringAllocParse(model_name);
			std::transform(model_name.begin(), model_name.end(), model_name.begin(), ::tolower);

			SafeStringAllocParse(s);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);
		
			SafeVarAllocParse(uint8_t, unk);

			if(terrain->GetModels().count(model_name) == 0) {
				EQGModelLoader model_loader;
				std::shared_ptr<EQG::Geometry> m(new EQG::Geometry());
				m->SetName(model_name);
				if (model_loader.Load(archive, model_name + ".mod", m)) {
					terrain->GetModels()[model_name] = m;
				}
				else if (model_loader.Load(archive, model_name, m)) {
					terrain->GetModels()[model_name] = m;
				}
				else {
					m->GetMaterials().clear();
					m->GetPolygons().clear();
					m->GetVertices().clear();
					terrain->GetModels()[model_name] = m;
				}
			}

			std::shared_ptr<Placeable> p(new Placeable());
			p->SetName(model_name);
			p->SetFileName(model_name);
			p->SetLocation(0.0f, 0.0f, 0.0f);
			p->SetRotation(rot_x, rot_y, rot_z);
			p->SetScale(scale_x, scale_y, scale_z);

			//There's a lot of work with offsets here =/
			std::shared_ptr<PlaceableGroup> pg(new PlaceableGroup());
			pg->SetFromTOG(false);
			pg->SetLocation(x, y, z);

			float terrain_height = 0.0f;
			float adjusted_x = x;
			float adjusted_y = y;

			if(adjusted_x < 0)
				adjusted_x = adjusted_x + (-(int)(adjusted_x / (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile)) + 1) * (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);
			else
				adjusted_x = fmod(adjusted_x, terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);

			if(adjusted_y < 0)
				adjusted_y = adjusted_y + (-(int)(adjusted_y / (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile)) + 1) * (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);
			else
				adjusted_y = fmod(adjusted_y, terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);

			int row_number = (int)(adjusted_y / terrain->GetOpts().units_per_vert);
			int column = (int)(adjusted_x / terrain->GetOpts().units_per_vert);
			int quad = row_number * terrain->GetOpts().quads_per_tile + column;

			float quad_vertex1Z = tile->GetFloats()[quad + row_number];
			float quad_vertex2Z = tile->GetFloats()[quad + row_number + terrain->GetOpts().quads_per_tile + 1];
			float quad_vertex3Z = tile->GetFloats()[quad + row_number + terrain->GetOpts().quads_per_tile + 2];
			float quad_vertex4Z = tile->GetFloats()[quad + row_number + 1];

			glm::vec3 p1(row_number * terrain->GetOpts().units_per_vert, (quad % terrain->GetOpts().quads_per_tile) * terrain->GetOpts().units_per_vert, quad_vertex1Z);
			glm::vec3 p2(p1.x + terrain->GetOpts().units_per_vert, p1.y, quad_vertex2Z);
			glm::vec3 p3(p1.x + terrain->GetOpts().units_per_vert, p1.y + terrain->GetOpts().units_per_vert, quad_vertex3Z);
			glm::vec3 p4(p1.x, p1.y + terrain->GetOpts().units_per_vert, quad_vertex4Z);
			
			terrain_height = HeightWithinQuad(p1, p2, p3, p4, adjusted_y, adjusted_x);

			pg->SetTileLocation(tile_start_y, tile_start_x, terrain_height);
			pg->SetRotation(0.0f, 0.0f, 0.0f);
			pg->SetScale(1.0f, 1.0f, 1.0f);
			pg->AddPlaceable(p);
			terrain->AddPlaceableGroup(pg);
		}

		SafeVarAllocParse(uint32_t, areas_count);
		for (uint32_t j = 0; j < areas_count; ++j) {
			SafeStringAllocParse(s);
			SafeVarAllocParse(int32_t, type);
			SafeStringAllocParse(s2);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);

			SafeVarAllocParse(float, size_x);
			SafeVarAllocParse(float, size_y);
			SafeVarAllocParse(float, size_z);

			std::shared_ptr<EQG::Region> region(new EQG::Region());

			float terrain_height = 0.0f;
			float adjusted_x = x;
			float adjusted_y = y;

			if (adjusted_x < 0)
				adjusted_x = adjusted_x + (-(int)(adjusted_x / (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile)) + 1) * (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);
			else
				adjusted_x = fmod(adjusted_x, terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);

			if (adjusted_y < 0)
				adjusted_y = adjusted_y + (-(int)(adjusted_y / (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile)) + 1) * (terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);
			else
				adjusted_y = fmod(adjusted_y, terrain->GetOpts().units_per_vert * terrain->GetOpts().quads_per_tile);

			int row_number = (int)(adjusted_y / terrain->GetOpts().units_per_vert);
			int column = (int)(adjusted_x / terrain->GetOpts().units_per_vert);
			int quad = row_number * terrain->GetOpts().quads_per_tile + column;

			float quad_vertex1Z = tile->GetFloats()[quad + row_number];
			float quad_vertex2Z = tile->GetFloats()[quad + row_number + terrain->GetOpts().quads_per_tile + 1];
			float quad_vertex3Z = tile->GetFloats()[quad + row_number + terrain->GetOpts().quads_per_tile + 2];
			float quad_vertex4Z = tile->GetFloats()[quad + row_number + 1];

			glm::vec3 p1(row_number * terrain->GetOpts().units_per_vert, (quad % terrain->GetOpts().quads_per_tile) * terrain->GetOpts().units_per_vert, quad_vertex1Z);
			glm::vec3 p2(p1.x + terrain->GetOpts().units_per_vert, p1.y, quad_vertex2Z);
			glm::vec3 p3(p1.x + terrain->GetOpts().units_per_vert, p1.y + terrain->GetOpts().units_per_vert, quad_vertex3Z);
			glm::vec3 p4(p1.x, p1.y + terrain->GetOpts().units_per_vert, quad_vertex4Z);

			terrain_height = HeightWithinQuad(p1, p2, p3, p4, adjusted_y, adjusted_x);

			region->SetName(s);
			region->SetAlternateName(s2);
			region->SetLocation(x + tile_start_y, y + tile_start_x, z + terrain_height);
			region->SetRotation(rot_x, rot_y, rot_z);
			region->SetScale(scale_x, scale_y, scale_z);
			region->SetExtents(size_x / 2.0f, size_y / 2.0f, size_z / 2.0f);
			region->SetFlags(type, 0);

			terrain->AddRegion(region);
		}

		SafeVarAllocParse(uint32_t, Light_effects_count);
		for (uint32_t j = 0; j < Light_effects_count; ++j) {
			SafeStringAllocParse(s);
			SafeStringAllocParse(s2);

			SafeVarAllocParse(int8_t, unk);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);

			SafeVarAllocParse(float, unk_float);
		}

		SafeVarAllocParse(uint32_t, tog_ref_count);
		for (uint32_t j = 0; j < tog_ref_count; ++j) {
			SafeStringAllocParse(tog_name);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);

			SafeVarAllocParse(float, z_adjust);

			std::vector<char> tog_buffer;
			if(!archive.Get(tog_name + ".tog", tog_buffer))
			{
				eqLogMessage(LogWarn, "Failed to load tog file %s.tog.", tog_name.c_str());
				continue;
			} else {
				eqLogMessage(LogTrace, "Loaded tog file %s.tog.", tog_name.c_str());
			}

			std::shared_ptr<PlaceableGroup> pg(new PlaceableGroup());
			pg->SetFromTOG(true);
			pg->SetLocation(x, y, z + (scale_z * z_adjust));
			pg->SetRotation(rot_x, rot_y, rot_z);
			pg->SetScale(scale_x, scale_y, scale_z);
			pg->SetTileLocation(tile_start_y, tile_start_x, 0.0f);

			std::vector<std::string> tokens;
			std::shared_ptr<Placeable> p;
			ParseConfigFile(tog_buffer, tokens);
			for (size_t k = 0; k < tokens.size();) {
				auto token = tokens[k];
				if (token.compare("*BEGIN_OBJECT") == 0) {
					p.reset(new Placeable());
					++k;
				}
				else if (token.compare("*NAME") == 0) {
					if (k + 1 >= tokens.size()) {
						break;
					}

					std::string model_name = tokens[k + 1];
					std::transform(model_name.begin(), model_name.end(), model_name.begin(), ::tolower);

					if (terrain->GetModels().count(model_name) == 0) {
						EQGModelLoader model_loader;
						std::shared_ptr<EQG::Geometry> m(new EQG::Geometry());
						m->SetName(model_name);
						if (model_loader.Load(archive, model_name + ".mod", m)) {
							terrain->GetModels()[model_name] = m;
						}
						else if (model_loader.Load(archive, model_name, m)) {
							terrain->GetModels()[model_name] = m;
						}
						else {
							m->GetMaterials().clear();
							m->GetPolygons().clear();
							m->GetVertices().clear();
							terrain->GetModels()[model_name] = m;
						}
					}

					p->SetName(model_name);
					p->SetFileName(model_name);
					k += 2;
				}
				else if (token.compare("*POSITION") == 0) {
					if (k + 3 >= tokens.size()) {
						break;
					}

					p->SetLocation(std::stof(tokens[k + 1]), std::stof(tokens[k + 2]), std::stof(tokens[k + 3]));
					k += 4;
				}
				else if (token.compare("*ROTATION") == 0) {
					if (k + 3 >= tokens.size()) {
						break;
					}

					p->SetRotation(std::stof(tokens[k + 1]), std::stof(tokens[k + 2]), std::stof(tokens[k + 3]));
					k += 4;
				}
				else if (token.compare("*SCALE") == 0) {
					if (k + 1 >= tokens.size()) {
						break;
					}

					p->SetScale(std::stof(tokens[k + 1]), std::stof(tokens[k + 1]), std::stof(tokens[k + 1]));
					k += 2;
				}
				else if (token.compare("*END_OBJECT") == 0) {
					pg->AddPlaceable(p);
					++k;
				}
				else {
					++k;
				}
			}

			terrain->AddPlaceableGroup(pg);
		}

		tile->SetLocation(tile_start_x, tile_start_y);
	}

	return true;
}

bool EQEmu::EQG4Loader::ParseWaterDat(EQEmu::PFS::Archive &archive, std::shared_ptr<EQG::Terrain> &terrain) {
	std::vector<char> wat;
	if(!archive.Get("water.dat", wat)) {
		return false;
	}

	std::vector<std::string> tokens;
	ParseConfigFile(wat, tokens);

	std::shared_ptr<EQG::WaterSheet> ws;

	for (size_t i = 1; i < tokens.size();) {
		auto token = tokens[i];
		if (token.compare("*WATERSHEET") == 0) {
			ws.reset(new EQG::WaterSheet());
			ws->SetTile(false);

			++i;
		}
		else if (token.compare("*END_SHEET") == 0) {
			if(ws) {
				eqLogMessage(LogTrace, "Adding finite water sheet.");
				terrain->AddWaterSheet(ws);
			}

			++i;
		}
		else if (token.compare("*WATERSHEETDATA") == 0) {
			ws.reset(new EQG::WaterSheet());
			ws->SetTile(true);
		
			++i;
		}
		else if (token.compare("*ENDWATERSHEETDATA") == 0) {
			if (ws) {
				eqLogMessage(LogTrace, "Adding infinite water sheet.");
				terrain->AddWaterSheet(ws);
			}
		
			++i;
		}
		else if (token.compare("*INDEX") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			int32_t index = std::stoi(tokens[i + 1]);
			ws->SetIndex(index);
			i += 2;
		}
		else if (token.compare("*MINX") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float min_x = std::stof(tokens[i + 1]);
			ws->SetMinX(min_x);

			i += 2;
		} else if (token.compare("*MINY") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float min_y = std::stof(tokens[i + 1]);
			ws->SetMinY(min_y);

			i += 2;
		} else if (token.compare("*MAXX") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float max_x = std::stof(tokens[i + 1]);
			ws->SetMaxX(max_x);

			i += 2;
		} else if (token.compare("*MAXY") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float max_y = std::stof(tokens[i + 1]);
			ws->SetMaxY(max_y);

			i += 2;
		} else if (token.compare("*ZHEIGHT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float z = std::stof(tokens[i + 1]);
			ws->SetZHeight(z);

			i += 2;
		}
		else {
			++i;
		}
	}

	return true;
}

bool EQEmu::EQG4Loader::ParseInvwDat(EQEmu::PFS::Archive &archive, std::shared_ptr<EQG::Terrain> &terrain) {
	std::vector<char> invw;
	if (!archive.Get("invw.dat", invw)) {
		return false;
	}

	char *buf = &invw[0];
	uint32_t count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	for(uint32_t i = 0; i < count; ++i) {
		std::string name = buf;
		buf += name.length() + 1;

		uint32_t flag = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		uint32_t vert_count = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		std::shared_ptr<EQG::InvisWall> w(new EQG::InvisWall());
		w->SetName(name);
		auto &verts = w->GetVerts();

		verts.resize(vert_count);
		for(uint32_t j = 0; j < vert_count; ++j) {
			float x = *(float*)buf;
			buf += sizeof(float);

			float y = *(float*)buf;
			buf += sizeof(float);

			float z = *(float*)buf;
			buf += sizeof(float);

			verts[j].x = x;
			verts[j].y = y;
			verts[j].z = z;			
		}

		terrain->AddInvisWall(w);
	}

	return true;
}

bool EQEmu::EQG4Loader::GetZon(std::string file, std::vector<char> &buffer) {
	buffer.clear();
	FILE *f = fopen(file.c_str(), "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		size_t sz = ftell(f);
		rewind(f);

		if (sz != 0) {
			buffer.resize(sz);

			size_t bytes_read = fread(&buffer[0], 1, sz, f);

			if (bytes_read != sz) {
				fclose(f);
				return false;
			}
			fclose(f);
		}
		else {
			fclose(f);
			return false;
		}
		return true;
	}
	return false;
}

void EQEmu::EQG4Loader::ParseConfigFile(std::vector<char> &buffer, std::vector<std::string> &tokens) {
	tokens.clear();
	std::string cur;
	for (size_t i = 0; i < buffer.size(); ++i) {
		char c = buffer[i];
		if(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f') {
			if(cur.size() > 0) {
				tokens.push_back(cur);
				cur.clear();
			}
		} else {
			cur.push_back(c);
		}
	}
}

bool EQEmu::EQG4Loader::ParseZon(std::vector<char> &buffer, EQG::Terrain::ZoneOptions &opts) {
	if (buffer.size() < 5)
		return false;

	std::vector<std::string> tokens;
	ParseConfigFile(buffer, tokens);

	if(tokens.size() < 1) {
		return false;
	}

	if(tokens[0].compare("EQTZP") != 0) {
		return 0;
	}

	for (size_t i = 1; i < tokens.size();) {
		auto token = tokens[i];
		if (token.compare("*NAME") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.name = tokens[i + 1];
			i += 2;
		}
		else if (token.compare("*MINLNG") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.min_lng = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MAXLNG") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.max_lng = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MINLAT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.min_lat = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MAXLAT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.max_lat = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MIN_EXTENTS") == 0) {
			if (i + 3 >= tokens.size()) {
				break;
			}

			opts.min_extents[0] = std::stof(tokens[i + 1]);
			opts.min_extents[1] = std::stof(tokens[i + 2]);
			opts.min_extents[2] = std::stof(tokens[i + 3]);
			i += 4;
		}
		else if (token.compare("*MAX_EXTENTS") == 0) {
			if (i + 3 >= tokens.size()) {
				break;
			}

			opts.max_extents[0] = std::stof(tokens[i + 1]);
			opts.max_extents[1] = std::stof(tokens[i + 2]);
			opts.max_extents[2] = std::stof(tokens[i + 3]);
			i += 4;
		}
		else if (token.compare("*UNITSPERVERT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.units_per_vert = std::stof(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*QUADSPERTILE") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.quads_per_tile = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*COVERMAPINPUTSIZE") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.cover_map_input_size = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*LAYERINGMAPINPUTSIZE") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.layer_map_input_size = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else {
			++i;
		}
	}

	return true;
}
